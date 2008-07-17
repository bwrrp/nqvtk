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

signals:
	void fpsChanged(int fps);

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
};
