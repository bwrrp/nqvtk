#include "GLBlaat/GL.h"

#include "NQVTKWidget.h"
#include "NQVTKWidget.moc"

#include "Math/Vector3.h"

#include "Rendering/Renderer.h"
#include "Rendering/OverlayRenderer.h"

#include <QMouseEvent>
#include <QGLFormat>

#include <algorithm>

// ----------------------------------------------------------------------------
NQVTKWidget::NQVTKWidget(QWidget *parent, const QGLWidget *shareWidget)
: QGLWidget(QGLFormat(QGL::AlphaChannel), parent, shareWidget), 
renderer(0), initialized(false), interactor(0)
{
	setMouseTracking(true);
	crosshairOn = false;
}

// ----------------------------------------------------------------------------
NQVTKWidget::~NQVTKWidget()
{
	if (interactor) delete interactor;
	if (renderer) 
	{
		makeCurrent();
		delete renderer;
	}
}

// ----------------------------------------------------------------------------
NQVTK::Renderer *NQVTKWidget::GetRenderer(bool getInner)
{
	if (getInner)
	{
		// If the renderer is an OverlayRenderer, get the base renderer
		NQVTK::OverlayRenderer *oren = dynamic_cast<NQVTK::OverlayRenderer*>(renderer);
		if (oren) return oren->GetBaseRenderer();
	}

	return renderer;
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
	else
	{
		qDebug("Init done!");
	}
}

// ----------------------------------------------------------------------------
void NQVTKWidget::resizeGL(int w, int h)
{
	if (initialized)
	{
		renderer->Resize(w, h);
		emit cameraUpdated(GetRenderer()->GetCamera());
	}
}

// ----------------------------------------------------------------------------
void NQVTKWidget::paintGL()
{
	// Draw if we can, otherwise just clear the screen
	if (initialized)
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
	// Pass the event to the interactor
	if (interactor)
	{
		if (interactor->MouseMoveEvent(event))
		{
			// TODO: this should probably not be emitted for each mouse event
			emit cameraUpdated(GetRenderer()->GetCamera());
			event->accept();
			updateGL();
		}
		else
		{
			event->ignore();
		}
	}

	// Compute projection-netral coordinates
	double x = (2.0 * static_cast<double>(event->x()) / this->width() - 1.0) * renderer->GetCamera()->aspect;
	double y = 2.0 * static_cast<double>(event->y()) / this->height() - 1.0;
	emit cursorPosChanged(x, y);
}
