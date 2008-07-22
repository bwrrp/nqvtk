#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Math/Vector3.h"

#include "Rendering/LayeredRenderer.h"

#include "Rendering/OrbitCamera.h"

#include <QMouseEvent>
#include <QGLFormat>

#include <algorithm>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent, const QGLWidget *shareWidget)
: QGLWidget(QGLFormat(QGL::AlphaChannel), parent, shareWidget), 
renderer(0), initialized(false)
{
	setMouseTracking(true);
	crosshairOn = false;
}

// ----------------------------------------------------------------------------
NQVTKWidget::~NQVTKWidget()
{
	if (renderer) 
	{
		makeCurrent();
		delete renderer;
	}
}

// ----------------------------------------------------------------------------
void NQVTKWidget::initializeGL()
{
	// TODO: add multi context support... GLBlaat should support this as well...
	if (glewInit() != GLEW_OK)
	{
		qDebug("Failed to initialize GLEW!");
		return;
	}

	qDebug("Initializing renderer...");
	if (renderer)
	{
		initialized = renderer->Initialize();
	}

	if (!initialized)
	{
		qDebug("Failed to initialize renderer...");
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
	// Draw if we can, otherwise just clear the screen
	if (initialized && renderer->GetNumberOfRenderables() > 0) 
	{
		renderer->Draw();
	}
	else
	{
		renderer->Clear();
	}

	// Draw crosshairs
	if (crosshairOn)
	{
		double x = (crosshairX / renderer->GetCamera()->aspect);
		double y = -crosshairY;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);

		glBegin(GL_LINES);
		glColor4d(1.0, 0.0, 0.0, 0.7);
		glVertex2d(x, -1.0);
		glVertex2d(x, 1.0);
		glVertex2d(-1.0, y);
		glVertex2d(1.0, y);
		glEnd();

		glPopAttrib();
	}
	
	// Increase fps count
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
void NQVTKWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier 
		|| event->modifiers() & Qt::AltModifier) 
	{
		NQVTK::Renderable *renderable;
		if (event->modifiers() & Qt::ControlModifier)
		{
			// Mouse controls object 0
			renderable = renderer->GetRenderable(0);
		}
		else
		{
			// Mouse controls clipper object (if it's there)
			renderable = renderer->GetRenderable(2);
		}

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
		NQVTK::OrbitCamera *cam = 
			dynamic_cast<NQVTK::OrbitCamera*>(renderer->GetCamera());
		if (cam)
		{
			// Mouse controls camera
			if (event->buttons() & Qt::LeftButton)
			{
				// Rotate
				cam->rotateY += event->x() - lastX;
				cam->rotateX -= event->y() - lastY;
				if (cam->rotateX > 80.0) cam->rotateX = 80.0;
				if (cam->rotateX < -80.0) cam->rotateX = -80.0;
			}

			if (event->buttons() & Qt::RightButton)
			{
				// Zoom
				cam->zoom += (event->y() - lastY) * 0.01f;
				if (cam->zoom < 0.05) cam->zoom = 0.05;
				if (cam->zoom > 20.0) cam->zoom = 20.0;
			}

			cam->Update();
			emit cameraUpdated(cam);
		}
	}

	lastX = event->x();
	lastY = event->y();

	event->accept();

	// Compute projection-netral coordinates
	double x = (2.0 * static_cast<double>(event->x()) / this->width() - 1.0) * renderer->GetCamera()->aspect;
	double y = 2.0 * static_cast<double>(event->y()) / this->height() - 1.0;
	emit cursorPosChanged(x, y);
}
