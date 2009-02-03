#pragma once

#include "Rendering/Renderer.h"
#include "Interactors/Interactor.h"
#include <QGLWidget>

class NQVTKWidget : public QGLWidget 
{
	Q_OBJECT

public:
	// TODO: add full QGLWidget constructors
	NQVTKWidget(QWidget *parent = 0, const QGLWidget *shareWidget = 0);
	virtual ~NQVTKWidget();
	
	void SetRenderer(NQVTK::Renderer *renderer)
	{
		assert(!this->renderer);
		this->renderer = renderer;
	}

	void SetInteractor(NQVTK::Interactor *interactor)
	{
		assert(!this->interactor);
		this->interactor = interactor;
		interactor->ResizeEvent(this->width(), this->height());
	}

	NQVTK::Renderer *GetRenderer(bool getInner = true);

	void StartContinuousUpdate()
	{
		// Render-on-idle timer
		startTimer(0);
		// FPS display timer
		fpsTimerId = startTimer(1000);
		frames = 0;
	}

public slots:
	void toggleCrosshair(bool on)
	{ 
		crosshairOn = on; 
		updateGL();
	}
	void setCrosshairPos(double x, double y)
	{
		crosshairX = x; crosshairY = y;
		updateGL();
	}
	void syncCamera(NQVTK::Camera *cam)
	{
		if (!renderer) return;
		renderer->GetCamera()->position = cam->position;
		renderer->GetCamera()->focus = cam->focus;
		renderer->GetCamera()->up = cam->up;
		updateGL();
	}

signals:
	void fpsChanged(int fps);
	void cursorPosChanged(double x, double y);
	void cameraUpdated(NQVTK::Camera *cam);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void timerEvent(QTimerEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	// The renderer
	NQVTK::Renderer *renderer;
	bool initialized;

	// The interactor
	NQVTK::Interactor *interactor;

	// FPS measurement
	int frames;
	int fpsTimerId;

	// Crosshair
	bool crosshairOn;
	double crosshairX;
	double crosshairY;
};
