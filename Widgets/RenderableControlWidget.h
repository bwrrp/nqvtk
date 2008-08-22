#pragma once

#include <QWidget>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QVBoxLayout>

#include "NQVTKWidget.h"

#include "Rendering/SimpleRenderer.h"

#include "Rendering/PolyData.h"
#include "Rendering/ImageDataTexture3D.h"

#include "Math/Vector3.h"

#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataReader.h>

#include <string>

class RenderableControlWidget : public QWidget
{
	Q_OBJECT;

public:
	RenderableControlWidget(NQVTKWidget *mainView, QWidget *parent = 0) : QWidget(parent)
	{
		assert(mainView);
		this->mainView = mainView;

		renderable = 0;
		color = NQVTK::Vector3(1.0, 1.0, 1.0);

		// Create the layout
		QVBoxLayout *layout = new QVBoxLayout(this);
		layout->setMargin(0);
		layout->setSpacing(0);
		this->setLayout(layout);

		// Set up the renderer
		CreateRenderer();

		// Create the menu bar
		CreateMenus();
	}

	void LoadMesh(const std::string &filename)
	{
		if (renderable)
		{
			// TODO: remove renderable from views
			delete renderable;
		}

		qDebug("Loading mesh %s", filename.c_str());
		vtkSmartPointer<vtkXMLPolyDataReader> reader = 
			vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(filename.c_str());
		reader->Update();

		// Create renderable in the main view
		mainView->makeCurrent();
		NQVTK::Renderable *renderable = new NQVTK::PolyData(reader->GetOutput());
		renderable->color = color;
		mainView->GetRenderer()->AddRenderable(renderable);

		// Add renderable to the simple view
		if (simpleView->isSharing())
		{
			// GL resources are shared, just add the original renderable
			simpleView->GetRenderer()->AddRenderable(renderable);
		}
		else
		{
			qDebug("WARNING! NQVTKWidgets can't share GL resources!");
			// TODO: we need to manually sync transformations between these objects
			simpleView->makeCurrent();
			NQVTK::Renderable *obj2 = new NQVTK::PolyData(reader->GetOutput()); 
			obj2->color = color;
			simpleView->GetRenderer()->AddRenderable(obj2);
		}
	}

	void LoadDistanceField(const std::string &filename)
	{
		// TODO: if there is no renderable, return
		// TODO: load df with VTK
		// TODO: create DistanceFieldParamSet
		// TODO: assign to renderable
	}

public slots:
	void ResetTransform()
	{
		// TODO: reset the transform for our renderable
	}

protected:
	NQVTKWidget *mainView;
	NQVTKWidget *simpleView;

	NQVTK::Renderable *renderable;

	NQVTK::Vector3 color;

	void CreateRenderer()
	{
		// Create the NQVTKWidget
		simpleView = new NQVTKWidget(0, mainView);
		this->layout()->addWidget(simpleView);
		// Set up the renderer
		NQVTK::SimpleRenderer *simpleRen = new NQVTK::SimpleRenderer();
		simpleView->SetRenderer(simpleRen);
		// Connect signals
		connect(mainView, SIGNAL(cursorPosChanged(double, double)), 
			simpleView, SLOT(setCrosshairPos(double, double)));
		connect(mainView, SIGNAL(cameraUpdated(NQVTK::Camera*)), 
			simpleView, SLOT(syncCamera(NQVTK::Camera*)));
		connect(simpleView, SIGNAL(cameraUpdated(NQVTK::Camera*)), 
			mainView, SLOT(updateGL()));
		// Set params
		simpleView->toggleCrosshair(true);
	}

	void CreateMenus()
	{
		QMenuBar *menuBar = new QMenuBar(this);
		this->layout()->setMenuBar(menuBar);

		QMenu *loadMenu = new QMenu("Load", menuBar);
		menuBar->addMenu(loadMenu);
		loadMenu->addAction("Mesh...", 
			this, SLOT(on_loadMesh_triggered()));
		loadMenu->addAction("Distance field...", 
			this, SLOT(on_loadDistanceField_triggered()));

		QMenu *paramMenu = new QMenu("Params", menuBar);
		menuBar->addMenu(paramMenu);
		//paramMenu->addAction("Color...");
		paramMenu->addAction("PCA weights...", 
			this, SLOT(on_paramPCA_triggered()));
		paramMenu->addSeparator();
		paramMenu->addAction("Reset transform", 
			this, SLOT(ResetTransform()));
	}

protected slots:
	void on_loadMesh_triggered()
	{
		QString qfilename = QFileDialog::getOpenFileName(
			this, "Load mesh", QString(), "VTK XML PolyData (*.vtp)");
		// This returns a null string when cancelled
		if (!qfilename.isNull())
		{
			std::string filename = std::string(qfilename.toUtf8());
			LoadMesh(filename);
		}
	}

	void on_loadDistanceField_triggered()
	{
		// TODO: if we don't have a renderable, return
		// TODO: show open file dialog
		// TODO: if not cancelled, get file name, call LoadDistanceField
	}

	void on_paramPCA_triggered()
	{
		// TODO: show PCA params widget for our renderable
	}
};
