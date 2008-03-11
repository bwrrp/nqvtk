#pragma once

#include "Rendering/Renderer.h"
#include <QGLWidget>

class NQVTKWidget : public QGLWidget {
	Q_OBJECT

public:
	// TODO: add full QGLWidget constructors
	NQVTKWidget(QWidget *parent = 0);
	virtual ~NQVTKWidget();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void timerEvent(QTimerEvent *event);
	void keyPressEvent(QKeyEvent *event);
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
