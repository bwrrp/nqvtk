#pragma once

#include <QWidget>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QSlider>
#include <QVBoxLayout>

#include "NQVTKWidget.h"

#include "Rendering/DistanceFieldParamSet.h"
#include "Rendering/ImageDataTexture3D.h"
#include "Rendering/PCAParamSet.h"
#include "Rendering/PolyData.h"
#include "Rendering/SimpleRenderer.h"

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
		NQVTK::PolyData *pd = new NQVTK::PolyData(reader->GetOutput());
		renderable = pd;
		renderable->color = color;
		renderable->opacity = opacity;
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
		mainView->makeCurrent();

		// Add other param sets
		// - PCA
		// TODO: also deform objects in simpleViews?
		int numEigenModes = NQVTK::PCAParamSet::GetNumEigenModes(pd);
		if (numEigenModes > 0)
		{
			NQVTK::PCAParamSet *pcaps = new NQVTK::PCAParamSet(numEigenModes);
			pd->SetParamSet("pca", pcaps);
		}

		// TODO: update interactor
		//simpleView->SetInteractor(new NQVTK::MainViewInteractor(simpleRen));
	}

	void LoadDistanceField(const std::string &filename)
	{
		if (!renderable) return;
		// Is there an old distance field?
		NQVTK::DistanceFieldParamSet *dfps = 
			dynamic_cast<NQVTK::DistanceFieldParamSet*>(
				renderable->GetParamSet("distancefield"));
		if (dfps) delete dfps;
		
		// Load the distance field
		vtkSmartPointer<vtkXMLImageDataReader> reader = 
			vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(filename.c_str());
		reader->Update();
		NQVTK::ImageDataTexture3D *tex = 
			NQVTK::ImageDataTexture3D::New(reader->GetOutput());
		assert(tex);
		// Assign to renderable
		dfps = new NQVTK::DistanceFieldParamSet(tex);
		renderable->SetParamSet("distancefield", dfps);
	}

	void SetColor(NQVTK::Vector3 color)
	{
		this->color = color;
		if (renderable)
		{
			renderable->color = color;
		}
	}

	void SetOpacity(double opacity)
	{
		this->opacity = opacity;
		if (renderable)
		{
			renderable->opacity = opacity;
		}
	}

public slots:
	void ResetTransform()
	{
		// TODO: transforms should probably use matrices to be more flexible
		renderable->position = NQVTK::Vector3();
		renderable->rotateX = 0.0;
		renderable->rotateY = 0.0;
	}

	void FocusObject()
	{
		if (!renderable) return;
		NQVTK::Camera *cam = mainView->GetRenderer()->GetCamera();
		cam->focus = renderable->GetCenter();
		// TODO: why doesn't this work?
	}

	void ShowPCAControls()
	{
		// Do we have a renderable?
		if (!renderable) return;
		// Is is a shape model?
		NQVTK::PCAParamSet *pcaps = dynamic_cast<NQVTK::PCAParamSet*>(
			renderable->GetParamSet("pca"));
		if (!pcaps) return;
		
		// Create PCA weight controls
		// TODO: add histograms above the sliders?
		QWidget *pcaWidget = new QWidget(
			mainView->topLevelWidget(), Qt::Tool);
		pcaWidget->setWindowTitle("Shape model weights");
		pcaWidget->resize(250, 50);
		pcaWidget->setStyleSheet(QString(
			"QSlider::groove:horizontal {"
			"	border: 1px solid #999999;"
			"	height: 1px;"
			"	background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
			"	margin: 3px 0;"
			"}"
			"QSlider::handle:horizontal {"
			"	border: 1px solid #5c5c5c;"
			"	width: 8px;"
			"	margin: -3px 0;"
			"   border-radius: 3px;"
			"   background: rgb(%1, %2, %3);"
			"}").arg(255 * color.x).arg(255 * color.y).arg(255 * color.z));
		QVBoxLayout *pcaLayout = new QVBoxLayout(pcaWidget);
		pcaWidget->setLayout(pcaLayout);
		for (unsigned int i = 0; i < pcaps->weights.size(); ++i)
		{
			QSlider *slider = new QSlider(pcaWidget);
			slider->setRange(-300, 300);
			slider->setValue(0);
			slider->setOrientation(Qt::Horizontal);
			slider->setMaximumHeight(7);
			slider->setProperty("modeId", static_cast<int>(i));
			// TODO: color slider handles to link visually to the renderable
			connect(slider, SIGNAL(valueChanged(int)), 
				this, SLOT(pcaSlider_valueChanged(int)));
			pcaLayout->addWidget(slider);
		}
		pcaWidget->show();
	}

protected:
	NQVTKWidget *mainView;
	NQVTKWidget *simpleView;

	NQVTK::Renderable *renderable;

	NQVTK::Vector3 color;
	double opacity;

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
			this, SLOT(loadMesh_triggered()));
		loadMenu->addAction("Distance field...", 
			this, SLOT(loadDistanceField_triggered()));

		QMenu *paramMenu = new QMenu("Params", menuBar);
		menuBar->addMenu(paramMenu);
		//paramMenu->addAction("Color...");
		paramMenu->addAction("PCA weights...", 
			this, SLOT(ShowPCAControls()));
		paramMenu->addSeparator();
		paramMenu->addAction("Reset transform", 
			this, SLOT(ResetTransform()));
		paramMenu->addAction("Focus object", 
			this, SLOT(FocusObject()));
	}

protected slots:
	void loadMesh_triggered()
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

	void loadDistanceField_triggered()
	{
		if (!renderable) return;
		QString qfilename = QFileDialog::getOpenFileName(
			this, "Load distance field", QString(), "VTK XML ImageData (*.vti)");
		// This returns a null string when cancelled
		if (!qfilename.isNull())
		{
			std::string filename = std::string(qfilename.toUtf8());
			LoadDistanceField(filename);
		}
	}

	void pcaSlider_valueChanged(int val)
	{
		int modeId = sender()->property("modeId").toInt();
		if (renderable)
		{
			NQVTK::PCAParamSet *pcaParams = dynamic_cast<NQVTK::PCAParamSet*>(
				renderable->GetParamSet("pca"));
			if (pcaParams)
			{
				pcaParams->weights[modeId] = static_cast<float>(val) / 100.0;
				mainView->updateGL();
			}
		}
	}
};
