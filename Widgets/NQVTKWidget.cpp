#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Rendering/PolyData.h"

#include <QTime>
#include <QKeyEvent>
#include <QApplication>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent /* = 0 */)
: QGLWidget(parent), renderable(0)
{	
}

// ----------------------------------------------------------------------------
NQVTKWidget::~NQVTKWidget()
{
	if (renderable) delete renderable;
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

	glClearColor(0.2f, 0.3f, 0.5f, 0.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	startTimer(0);
}

// ----------------------------------------------------------------------------
void NQVTKWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
		45.0, 
		static_cast<double>(w) / static_cast<double>(h), 
		150.0, 600.0);
	glMatrixMode(GL_MODELVIEW);
}

// ----------------------------------------------------------------------------
void NQVTKWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: replace with some camera abstraction
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		88.3, 109.2, 400.8, 
		88.3, 108.4, 78.2, 
		0.0, 1.0, 0.0);

	// Add some silly rotation
	// TODO: local transformations should be handled by the Renderable
	QTime midnight = QTime(0, 0);
	glTranslated(88.3, 108.4, 78.2);
	glRotated(static_cast<double>(QTime::currentTime().msecsTo(midnight)) / 10.0, 
		0.0, 1.0, 0.0);
	glTranslated(-88.3, -108.4, -78.2);

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
