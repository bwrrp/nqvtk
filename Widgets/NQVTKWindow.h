#pragma once

#include <QMainWindow>
#include <QString>

#include "ui_NQVTKWindow.h"

#include "Styles/DistanceFields.h"
#include "Rendering/CrossEyedStereoRenderer.h"

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
		ui.nqvtkwidget->distfieldStyle->useGridTexture = val;
	}

	void on_useGlyphTexture_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldStyle->useGlyphTexture = val;
	}

	void on_classificationThreshold_valueChanged(int val) 
	{
		ui.nqvtkwidget->distfieldStyle->classificationThreshold = 
			static_cast<double>(val) / 100.0;
	}

	void on_useDistanceColorMap_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldStyle->useDistanceColorMap = val;
	}

	void on_contourDepthEpsilon_valueChanged(int val) 
	{
		ui.nqvtkwidget->distfieldStyle->contourDepthEpsilon = 
			static_cast<double>(val) / 1000.0;
	}

	void on_useFatContours_toggled(bool val) 
	{ 
		ui.nqvtkwidget->distfieldStyle->useFatContours = val;
	}

	void on_depthCueRange_valueChanged(int val) 
	{
		ui.nqvtkwidget->distfieldStyle->depthCueRange = 
			static_cast<double>(val) / 1000.0;
	}

	void on_useFog_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldStyle->useFog = val;
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

	void on_maxLayers_valueChanged(int val)
	{
		NQVTK::LayeredRenderer *renderer = 
			dynamic_cast<NQVTK::LayeredRenderer*>(ui.nqvtkwidget->GetRenderer());
		if (renderer) renderer->maxLayers = val;
	}

	void on_eyeSpacing_valueChanged(int val)
	{
		// Do we have a stereo renderer?
		NQVTK::CrossEyedStereoRenderer *renderer = 
			dynamic_cast<NQVTK::CrossEyedStereoRenderer*>(ui.nqvtkwidget->GetRenderer());
		if (renderer) stereoRenderer->eyeSpacing = 
			static_cast<double>(val) / 1000.0;
	}

	void on_lightOffsetDirection_valueChanged(int val)
	{
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		renderer->lightOffsetDirection = val;
	}

	void on_lightRelativeToCamera_toggled(bool val)
	{
		ui.lightOffsetDirection->setEnabled(val);
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		renderer->lightRelativeToCamera = val;
	}
};
