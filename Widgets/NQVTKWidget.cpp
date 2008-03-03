#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Rendering/PolyData.h"
#include "Rendering/Camera.h"

#include <QTime>
#include <QKeyEvent>
#include <QApplication>

#include <cmath>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent /* = 0 */)
: QGLWidget(parent), renderable(0)
{	
}

// ----------------------------------------------------------------------------
NQVTKWidget::~NQVTKWidget()
{
	if (renderable) delete renderable;
	if (camera) delete camera;
}

// ----------------------------------------------------------------------------
void NQVTKWidget::initializeGL()
{

	if (glewInit() != GLEW_OK)
	{
		qDebug("Failed to initialize GLEW!");
		return;
	}

	qDebug("Creating renderable...");
	if (renderable) delete renderable;
	renderable = new NQVTK::PolyData(
		"D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2.vtp");

	qDebug("Creating camera...");
	if (camera) delete camera;
	camera = new NQVTK::Camera();

	glClearColor(0.2f, 0.3f, 0.5f, 0.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	startTimer(0);

	qDebug("Init done!");
}

// ----------------------------------------------------------------------------
void NQVTKWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));

	camera->aspect = static_cast<double>(w) / static_cast<double>(h);
}

// ----------------------------------------------------------------------------
void NQVTKWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get model information
	double cx, cy, cz;
	renderable->GetCenter(cx, cy, cz);
	double bounds[6];
	renderable->GetBounds(bounds);

	// Setup camera (matrices)
	// TODO: add a useful way to focus camera on objects
	camera->FocusOn(renderable);
	camera->Draw();

	// Add some silly rotation
	// TODO: local transformations should be handled by the Renderable
	QTime midnight = QTime(0, 0);
	glRotated(static_cast<double>(QTime::currentTime().msecsTo(midnight)) / 30.0, 
		0.0, 1.0, 0.0);
	glTranslated(-cx, -cy, -cz);

	renderable->Draw();
}

// ----------------------------------------------------------------------------
void NQVTKWidget::timerEvent(QTimerEvent *event)
{
	// Update on idle
	updateGL();
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
