#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Rendering/Renderer.h"
#include "Rendering/PolyData.h"

#include <vtkXMLPolyDataReader.h>
#include <vtkSmartPointer.h>

#include <QKeyEvent>
#include <QApplication>
#include <QGLFormat>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent /* = 0 */)
: QGLWidget(QGLFormat(QGL::AlphaChannel), parent), renderer(0), initialized(false)
{	
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
	initialized = renderer->Initialize();

	if (!initialized)
	{
		qDebug("Failed to initialize renderer...");
	}
	
	qDebug("Creating and adding renderable...");
	// Load a polydata for testing
	vtkSmartPointer<vtkXMLPolyDataReader> reader = 
		vtkSmartPointer<vtkXMLPolyDataReader>::New();
	reader->SetFileName(
		"D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2.vtp");
	reader->Update();
	qDebug("Loaded PolyData...");
	NQVTK::Renderable *renderable = new NQVTK::PolyData(reader->GetOutput());
	renderer->AddRenderable(renderable);

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
	renderer->Clear();
	if (initialized) 
	{
		renderer->Draw();
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
	if (event->key() == Qt::Key_Escape)
		qApp->quit();
	else
		event->ignore();
}
