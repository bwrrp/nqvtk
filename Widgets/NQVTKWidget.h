#pragma once

#include "Rendering/Renderer.h"
#include <QGLWidget>

#include "Styles/DepthPeeling.h"
#include "Styles/IBIS.h"
#include "Styles/DistanceFields.h"

class NQVTKWidget : public QGLWidget {
	Q_OBJECT

public:
	// TODO: add full QGLWidget constructors
	NQVTKWidget(QWidget *parent = 0);
	virtual ~NQVTKWidget();

	// HACK: exposed this way for now... improve later...
	NQVTK::Styles::DepthPeeling *depthpeelStyle;
	NQVTK::Styles::IBIS *ibisStyle;
	NQVTK::Styles::DistanceFields *distfieldStyle;
	NQVTK::Renderer *GetRenderer() { return renderer; }

signals:
	void fpsChanged(int);

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
