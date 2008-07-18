#pragma once

#include "Rendering/Renderer.h"
#include <QGLWidget>

class NQVTKWidget : public QGLWidget {
	Q_OBJECT

public:
	// TODO: add full QGLWidget constructors
	NQVTKWidget(QWidget *parent = 0);
	virtual ~NQVTKWidget();
	
	void SetRenderer(NQVTK::Renderer *renderer)
	{
		assert(!this->renderer);
		this->renderer = renderer;
	}

	NQVTK::Renderer *GetRenderer() { return renderer; }

public slots:
	void toggleCrosshair(bool on) { crosshairOn = on; }
	void setCrosshairPos(double x, double y)
	{
		crosshairX = x; crosshairY = y;
	}

signals:
	void fpsChanged(int fps);
	void cursorPosChanged(double x, double y);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void timerEvent(QTimerEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	// The renderer
	NQVTK::Renderer *renderer;
	bool initialized;

	// FPS measurement
	int frames;
	int fpsTimerId;

	// Previous mouse coordinates
	int lastX;
	int lastY;

	// Crosshair
	bool crosshairOn;
	double crosshairX;
	double crosshairY;
};
