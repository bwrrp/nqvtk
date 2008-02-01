#pragma once

#include <QGLWidget>

#include "Rendering/Renderable.h"

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

	NQVTK::Renderable *renderable;
};
