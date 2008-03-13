#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Math/Vector3.h"

#include "Rendering/Renderer.h"
#include "Rendering/PolyData.h"
#include "Rendering/Camera.h"

#include "Styles/DepthPeeling.h"
#include "Styles/IBIS.h"
#include "Styles/DistanceFields.h"

#include <vtkXMLPolyDataReader.h>
#include <vtkSmartPointer.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStringList>
#include <QGLFormat>

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
	renderer->SetStyle(new NQVTK::Styles::IBIS());
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
		args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2.vtp");
		args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_08_02-T2.vtp");
		//*/
		//*  Cartilage
		args.append("D:/Data/Cartilage/Lorena/femoral2.vtp");
		args.append("D:/Data/Cartilage/Lorena/femoral1.vtp");
		//*/
	}
	// Load the polydata
	NQVTK::Vector3 colors[] = {
		NQVTK::Vector3(1.0, 0.9, 0.4), 
		NQVTK::Vector3(0.3, 0.6, 1.0)
	};
	int i = 0;
	for (QStringList::const_iterator it = args.begin() + 1; it != args.end(); ++it)
	{
		qDebug("Loading #%d...", i);
		vtkSmartPointer<vtkXMLPolyDataReader> reader = 
			vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(it->toUtf8());
		reader->Update();
		NQVTK::Renderable *renderable = new NQVTK::PolyData(reader->GetOutput());
		renderable->color = colors[std::min(i, 1)];
		renderable->opacity = 0.3;
		renderer->AddRenderable(renderable);
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
		setWindowTitle(QString("NQVTK - %1 FPS").arg(frames));
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
		renderer->SetStyle(new NQVTK::Styles::DepthPeeling());
		break;
	case Qt::Key_F2:
		renderer->SetStyle(new NQVTK::Styles::IBIS());
		break;
	case Qt::Key_F3:
		renderer->SetStyle(new NQVTK::Styles::DistanceFields());
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
		if (renderer->GetCamera()->zoom < -50.0) 
		{
			renderer->GetCamera()->zoom = -50.0;
		}
		if (renderer->GetCamera()->zoom > 50.0) 
		{
			renderer->GetCamera()->zoom = 50.0;
		}
	}

	lastX = event->x();
	lastY = event->y();

	event->accept();
}
