#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Rendering/PolyData.h"

#include <QTime>
#include <QKeyEvent>
#include <QApplication>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent /* = 0 */)
: QGLWidget(parent)
{	
	renderable = new NQVTK::PolyData;
}

// ----------------------------------------------------------------------------
NQVTKWidget::~NQVTKWidget()
{
	delete renderable;
}

// ----------------------------------------------------------------------------
void NQVTKWidget::initializeGL()
{
	glClearColor(0.2f, 0.3f, 0.5f, 0.0f);

	glDisable(GL_CULL_FACE);

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
		0.1, 100.0);
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
		0.0, 2.0, 10.0, 
		0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0);

	// Add some silly rotation
	// TODO: local transformations should be handled by the Renderable
	QTime midnight = QTime(0, 0);
	glRotated(static_cast<double>(QTime::currentTime().msecsTo(midnight)) / 10.0, 
		0.0, 1.0, 0.0);

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
