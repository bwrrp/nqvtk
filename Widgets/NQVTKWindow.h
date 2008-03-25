#pragma once

#include <QMainWindow>
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
	void on_useGridTexture_toggled(bool val) 
	{
		ui.nqvtkwidget->distfieldstyle->useGridTexture = val;
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
};
