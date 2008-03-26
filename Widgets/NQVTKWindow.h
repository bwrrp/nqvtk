#pragma once

#include <QMainWindow>
#include <QString>

#include "ui_NQVTKWindow.h"

#include "Styles/DistanceFields.h"

class NQVTKWindow : public QMainWindow
{
	Q_OBJECT;

public:
	NQVTKWindow(QWidget *parent = 0) : QMainWindow(parent) 
	{ 
		ui.setupUi(this);
	}

private:
	Ui::NQVTKWindow ui;

private slots:
	void on_nqvtkwidget_fpsChanged(int fps)
	{
		setWindowTitle(QString("NQVTK - %1 FPS @ %2x%3")
			.arg(fps)
			.arg(ui.nqvtkwidget->width())
			.arg(ui.nqvtkwidget->height()));
	}

	void on_useGridTexture_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldstyle->useGridTexture = val;
	}

	void on_useGlyphTexture_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldstyle->useGlyphTexture = val;
	}

	void on_classificationThreshold_valueChanged(int val) 
	{
		ui.nqvtkwidget->distfieldstyle->classificationThreshold = 
			static_cast<double>(val) / 100.0;
	}

	void on_useDistanceColorMap_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldstyle->useDistanceColorMap = val;
	}

	void on_contourDepthEpsilon_valueChanged(int val) 
	{
		ui.nqvtkwidget->distfieldstyle->contourDepthEpsilon = 
			static_cast<double>(val) / 1000.0;
	}

	void on_useFatContours_toggled(bool val) 
	{ 
		ui.nqvtkwidget->distfieldstyle->useFatContours = val;
	}

	void on_depthCueRange_valueChanged(int val) 
	{
		ui.nqvtkwidget->distfieldstyle->depthCueRange = 
			static_cast<double>(val) / 100.0;
	}

	void on_useFog_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldstyle->useFog = val;
	}

	void on_opacity_valueChanged(int val)
	{
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		for (int i = 0; i < renderer->GetNumberOfRenderables(); ++i)
		{
			renderer->GetRenderable(i)->opacity = 
				static_cast<double>(val) / 100.0;
		}
	}
};
