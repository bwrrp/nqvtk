#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Math/Vector3.h"

#include "Rendering/Renderer.h"
#include "Rendering/Camera.h"
#include "Rendering/PolyData.h"
#include "Rendering/ImageDataTexture3D.h"

#include "Styles/DepthPeeling.h"
#include "Styles/IBIS.h"
#include "Styles/DistanceFields.h"

#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataReader.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStringList>
#include <QGLFormat>
#include <QDateTime>

#include <algorithm>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent /* = 0 */)
: QGLWidget(QGLFormat(QGL::AlphaChannel), parent), renderer(0), initialized(false)
{
	setMouseTracking(true);
}

// ----------------------------------------------------------------------------
NQVTKWidget::~NQVTKWidget()
{
	if (renderer) delete renderer;
}

// ----------------------------------------------------------------------------
void NQVTKWidget::initializeGL()
{
	if (glewInit() != GLEW_OK)
	{
		qDebug("Failed to initialize GLEW!");
		return;
	}

	qDebug("Initializing renderer...");
	if (!renderer) renderer = new NQVTK::Renderer();
	NQVTK::Styles::DistanceFields *style = new NQVTK::Styles::DistanceFields();
	this->distfieldstyle = style;
	renderer->SetStyle(style);
	initialized = renderer->Initialize();

	if (!initialized)
	{
		qDebug("Failed to initialize renderer...");
	}
	
	qDebug("Creating and adding renderables...");
	QStringList args = qApp->arguments();
	if (args.size() < 2) 
	{
		qDebug("No arguments supplied, using default data...");
		// Set default testing data (on Vliet)
		/* - msdata
		args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2-textured.vtp");
		args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_08_02-T2-textured.vtp");
		args.append("-"); // distance field marker
		// NOTE: fields are flipped because object 1 should use distfield 0
		args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_08_02-T2-dist256.vti");
		args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2-dist256.vti");
		//*/
		/*  Cartilage
		args.append("D:/Data/Cartilage/Lorena/femoral2-textured.vtp");
		args.append("D:/Data/Cartilage/Lorena/femoral1-textured.vtp");
		args.append("-");
		args.append("D:/Data/Cartilage/Lorena/femoral1-dist256.vti");
		args.append("D:/Data/Cartilage/Lorena/femoral2-dist256.vti");
		//*/
		/* Ventricles
		args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20040510_textured.vtp");
		args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20041124_textured.vtp");
		args.append("-"); // distance field marker
		args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20041124_padded.vti");
		args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20040510_padded.vti");
		//*/
		//* Dragon
		args.append("D:/Data/Surface/dragon-tri2.vtp");
		args.append("D:/Data/Surface/dragon-smooth-tri2.vtp");
		//*/
	}
	// Load the polydata
	NQVTK::Vector3 colors[] = {
		NQVTK::Vector3(1.0, 0.9, 0.4), 
		NQVTK::Vector3(0.3, 0.6, 1.0)
	};
	int i = 0;
	bool distFields = false;
	for (QStringList::const_iterator it = args.begin() + 1; it != args.end(); ++it)
	{
		qDebug("Loading %s #%d...", (distFields ? "distance field" : "surface"), i);
		if (QString("-") == *it)
		{
			distFields = true;
			i = 0;
			continue;
		}
		if (!distFields)
		{
			// Load surface
			vtkSmartPointer<vtkXMLPolyDataReader> reader = 
				vtkSmartPointer<vtkXMLPolyDataReader>::New();
			reader->SetFileName(it->toUtf8());
			reader->Update();
			NQVTK::Renderable *renderable = new NQVTK::PolyData(reader->GetOutput());
			renderable->color = colors[std::min(i, 1)];
			renderable->opacity = 0.3;
			renderer->AddRenderable(renderable);
		}
		else
		{
			// Load distance field
			vtkSmartPointer<vtkXMLImageDataReader> reader = 
				vtkSmartPointer<vtkXMLImageDataReader>::New();
			reader->SetFileName(it->toUtf8());
			reader->Update();
			GLTexture *tex = NQVTK::ImageDataTexture3D::New(reader->GetOutput());
			assert(tex);
			distfieldstyle->SetDistanceField(i, tex);
		}
		++i;
	}

	// Render-on-idle timer
	startTimer(0);
	// FPS display timer
	fpsTimerId = startTimer(1000);
	frames = 0;

	qDebug("Init done!");
}

// ----------------------------------------------------------------------------
void NQVTKWidget::resizeGL(int w, int h)
{
	renderer->Resize(w, h);
}

// ----------------------------------------------------------------------------
void NQVTKWidget::paintGL()
{
	if (initialized) 
	{
		renderer->Draw();
	}
	else
	{
		renderer->Clear();
	}
	++frames;
}

// ----------------------------------------------------------------------------
void NQVTKWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == fpsTimerId)
	{
		// Update FPS display
		emit fpsChanged(frames);
		frames = 0;
	}
	else
	{
		// Update on idle
		updateGL();
	}
}

// ----------------------------------------------------------------------------
void NQVTKWidget::keyPressEvent(QKeyEvent *event)
{
	// Quit on Escape
	switch (event->key())
	{
	case Qt::Key_Escape:
		qApp->quit();
		break;
	case Qt::Key_C:
		{
			QDateTime now = QDateTime::currentDateTime();
			QImage screenshot = this->grabFrameBuffer(true);
			// Fix alpha values
			screenshot.invertPixels(QImage::InvertRgba);
			screenshot.invertPixels(QImage::InvertRgb);
			// Save it
			screenshot.save(QString("NQVTK-%1.png").arg(
				now.toString("yyMMdd-hhmmss")), "PNG");
			break;
		}
	case Qt::Key_R:
		renderer->ResetRenderables();
		break;
	case Qt::Key_1:
		{
			NQVTK::Renderable *ren = renderer->GetRenderable(0);
			if (ren) ren->visible = !ren->visible;
		}
		break;
	case Qt::Key_2:
		{
			NQVTK::Renderable *ren = renderer->GetRenderable(1);
			if (ren) ren->visible = !ren->visible;
		}
		break;
	case Qt::Key_F1:
		initialized = renderer->SetStyle(new NQVTK::Styles::DepthPeeling());
		break;
	case Qt::Key_F2:
		initialized = renderer->SetStyle(new NQVTK::Styles::IBIS());
		break;
	case Qt::Key_F3:
		distfieldstyle = new NQVTK::Styles::DistanceFields();
		initialized = renderer->SetStyle(distfieldstyle);
		break;
	default:
		event->ignore();
		return;
	}
	event->accept();
}

// ----------------------------------------------------------------------------
void NQVTKWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier) 
	{
		NQVTK::Renderable *renderable = renderer->GetRenderable(0);
		// Mouse controls object 0
		if (renderable && event->buttons() & Qt::LeftButton)
		{
			// Rotate
			renderable->rotateY += event->x() - lastX;
			renderable->rotateX -= event->y() - lastY;

			if (renderable->rotateX > 80.0) 
			{
				renderable->rotateX = 80.0;
			}
			if (renderable->rotateX < -80.0) 
			{
				renderable->rotateX = -80.0;
			}
		}
		if (renderable && event->buttons() & Qt::MidButton)
		{
			// TODO: make translation relative to window
			NQVTK::Vector3 right = NQVTK::Vector3(1.0, 0.0, 0.0);
			NQVTK::Vector3 up = NQVTK::Vector3(0.0, 1.0, 0.0);
			double factor = 0.6;
			// Translate
			renderable->position += 
				(lastX - event->x()) * factor * right +
				(lastY - event->y()) * factor * up;
		}
	}
	else
	{
		// Mouse controls camera
		if (event->buttons() & Qt::LeftButton)
		{
			// Rotate
			renderer->GetCamera()->rotateY += event->x() - lastX;
			renderer->GetCamera()->rotateX -= event->y() - lastY;

			if (renderer->GetCamera()->rotateX > 80.0) 
			{
				renderer->GetCamera()->rotateX = 80.0;
			}
			if (renderer->GetCamera()->rotateX < -80.0) 
			{
				renderer->GetCamera()->rotateX = -80.0;
			}
		}

		if (event->buttons() & Qt::RightButton)
		{
			// Zoom
			renderer->GetCamera()->zoom += (event->y() - lastY) * 0.01f;
			if (renderer->GetCamera()->zoom < 0.05) 
			{
				renderer->GetCamera()->zoom = 0.05;
			}
			if (renderer->GetCamera()->zoom > 20.0) 
			{
				renderer->GetCamera()->zoom = 20.0;
			}
		}
	}

	lastX = event->x();
	lastY = event->y();

	event->accept();
}
