#pragma once

#include <QWidget>

#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "NQVTKWidget.h"

#include "Rendering/DistanceFieldParamSet.h"
#include "Rendering/ImageDataTexture3D.h"
#include "Rendering/PCAParamSet.h"
#include "Rendering/PolyData.h"
#include "Rendering/SimpleRenderer.h"
#include "Rendering/TransferFunctionParamSet.h"

#include "Math/Vector3.h"

#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataReader.h>

#include <string>

class PCASlider : public QWidget
{
	Q_OBJECT;

public:
	PCASlider(QWidget *parent = 0) : QWidget(parent)
	{
		// Create subwidgets
		QHBoxLayout *line = new QHBoxLayout(this);
		line->setMargin(0);
		line->setSpacing(2);
		// - Slider
		slider = new QSlider(this);
		slider->setRange(-300, 300);
		slider->setValue(0);
		slider->setOrientation(Qt::Horizontal);
		slider->setMaximumHeight(7);
		slider->setTickPosition(QSlider::TicksBothSides);
		slider->setTickInterval(50);
		connect(slider, SIGNAL(valueChanged(int)), 
			this, SLOT(slider_valueChanged(int)));
		line->addWidget(slider);
		// - Value readout
		label = new QLabel("0.0", this);
		label->setMaximumWidth(50);
		label->setMinimumWidth(50);
		line->addWidget(label);
		// - Reset button
		resetButton = new QPushButton("0", this);
		resetButton->setMaximumWidth(20);
		resetButton->setMinimumWidth(20);
		resetButton->setMaximumHeight(16);
		connect(resetButton, SIGNAL(clicked()), 
			this, SLOT(reset()));
		line->addWidget(resetButton);
	}

signals:
	void valueChanged(float);

public slots:
	void reset()
	{
		slider->setValue(0);
	}

	void setValue(float val)
	{
		slider->setValue(100 * val);
	}

private slots:
	void slider_valueChanged(int val)
	{
		float v = static_cast<float>(val) / 100.0;
		label->setText(QString("%1").arg(v, 0, 'f', 3));
		emit valueChanged(v);
	}

private:
	QSlider *slider;
	QLabel *label;
	QPushButton *resetButton;
};



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
		opacity = 1.0;

		// Create the layout
		QVBoxLayout *layout = new QVBoxLayout(this);
		layout->setMargin(0);
		layout->setSpacing(0);
		this->setLayout(layout);

		// Set up the renderer
		CreateRenderer();

		// Create the menu bar
		CreateMenus();

		// Add a renderable slot in the main view
		slot = mainView->GetRenderer()->AddRenderable(0);
	}

	void LoadMesh(const std::string &filename)
	{
		if (renderable)
		{
			// Delete old renderables
			NQVTK::Renderable *simpleObj = 
				simpleView->GetRenderer()->SetRenderable(0, 0);
			if (simpleObj != 0 && simpleObj != renderable) delete simpleObj;
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
		mainView->GetRenderer()->SetRenderable(slot, renderable);

		// Add renderable to the simple view
		if (simpleView->isSharing())
		{
			// GL resources are shared, just add the original renderable
			simpleView->GetRenderer()->SetRenderable(0, renderable);
		}
		else
		{
			qDebug("WARNING! NQVTKWidgets can't share GL resources!");
			// TODO: we need to manually sync transformations between these objects
			simpleView->makeCurrent();
			NQVTK::Renderable *obj2 = new NQVTK::PolyData(reader->GetOutput()); 
			obj2->color = color;
			simpleView->GetRenderer()->SetRenderable(0, obj2);
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

		// For now, also add a transfer function
		// TODO: this should probably be handled some other way
		NQVTK::TransferFunctionParamSet *tfps = 
			new NQVTK::TransferFunctionParamSet();
		//tfps->tfStart = tex->dataShift;
		//tfps->tfEnd = tex->dataScale + tex->dataShift;
		renderable->SetParamSet("transferfunction", tfps);
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

	void ShowTFWidget()
	{
		// Do we have a renderable?
		if (!renderable) return;
		// Does it have the right paramset?
		NQVTK::TransferFunctionParamSet *tfps = 
			dynamic_cast<NQVTK::TransferFunctionParamSet*>(
				renderable->GetParamSet("transferfunction"));
		if (!tfps) return;

		// Create transfer function widget
		// TODO: create a widget to define the tf lookup texture
		// Right now, this simply has two sliders for the start and end values
		QWidget *tfWidget = new QWidget(
			mainView->topLevelWidget(), Qt::Tool);
		tfWidget->setWindowTitle("Transfer function");
		tfWidget->resize(250, 50);
		QVBoxLayout *tfLayout = new QVBoxLayout(tfWidget);
		QSlider *tfStartSlider = new QSlider(tfWidget);
		// TODO: Range of the slider should be linked to the data
		tfStartSlider->setRange(0, 255);
		tfStartSlider->setValue(0);
		tfStartSlider->setOrientation(Qt::Horizontal);
		connect(tfStartSlider, SIGNAL(valueChanged(int)), 
			this, SLOT(tfStartSlider_valueChanged(int)));
		tfLayout->addWidget(tfStartSlider);
		QSlider *tfEndSlider = new QSlider(tfWidget);
		// TODO: Range of the slider should be linked to the data
		tfEndSlider->setRange(0, 255);
		tfEndSlider->setValue(255);
		tfEndSlider->setOrientation(Qt::Horizontal);
		connect(tfEndSlider, SIGNAL(valueChanged(int)), 
			this, SLOT(tfEndSlider_valueChanged(int)));
		tfLayout->addWidget(tfEndSlider);
		tfWidget->show();
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
		pcaWidget->resize(300, 50);
		pcaWidget->setStyleSheet(QString(
			"QSlider::groove:horizontal {"
			"	border: 1px solid #999999;"
			"	height: 1px;"
			"	background: qlineargradient(spread:pad, "
			"		x1:0, y1:0, x2:1, y2:0, "
			"		stop:0 rgba(220, 220, 220, 255), "
			"		stop:0.15 rgba(180, 180, 180, 255), "
			"		stop:0.16 rgba(160, 160, 160, 255), "
			"		stop:0.17 rgba(220, 220, 220, 255), "
			"		stop:0.32 rgba(180, 180, 180, 255), "
			"		stop:0.330479 rgba(160, 160, 160, 255), "
			"		stop:0.34 rgba(220, 220, 220, 255), "
			"		stop:0.49 rgba(180, 180, 180, 255), "
			"		stop:0.5 rgba(160, 160, 160, 255), "
			"		stop:0.51 rgba(220, 220, 220, 255), "
			"		stop:0.65 rgba(180, 180, 180, 255), "
			"		stop:0.659406 rgba(160, 160, 160, 255), "
			"		stop:0.67 rgba(220, 220, 220, 255), "
			"		stop:0.83 rgba(180, 180, 180, 255), "
			"		stop:0.84 rgba(160, 160, 160, 255), "
			"		stop:0.85 rgba(220, 220, 220, 255), "
			"		stop:1 rgba(180, 180, 180, 255));"
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
		// Weights reset button
		QPushButton *resetButton = new QPushButton(pcaWidget);
		resetButton->setText("Reset all weights");
		// Weight sliders
		for (unsigned int i = 0; i < pcaps->weights.size(); ++i)
		{
			// Create a slider
			PCASlider *slider = new PCASlider(pcaWidget);
			slider->setProperty("modeId", static_cast<int>(i));
			connect(slider, SIGNAL(valueChanged(float)), 
				this, SLOT(pcaSlider_valueChanged(float)));
			// Connect the main reset button
			connect(resetButton, SIGNAL(clicked()), 
				slider, SLOT(reset()));
			pcaLayout->addWidget(slider);
		}
		pcaLayout->addWidget(resetButton);
		pcaWidget->show();
	}

protected:
	NQVTKWidget *mainView;
	NQVTKWidget *simpleView;

	int slot;
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
		simpleView->syncCamera(mainView->GetRenderer()->GetCamera());
		// Add slot for the renderable
		simpleRen->AddRenderable(0);
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
		// TODO: create widgets and add menu items dynamically based on paramsets
		paramMenu->addAction("Transfer function...", 
			this, SLOT(ShowTFWidget()));
		paramMenu->addAction("PCA weights...", 
			this, SLOT(ShowPCAControls()));
		paramMenu->addSeparator();
		paramMenu->addAction("Reset transform", 
			this, SLOT(ResetTransform()));
		//paramMenu->addAction("Focus object", 
		//	this, SLOT(FocusObject()));
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

	void tfStartSlider_valueChanged(int val)
	{
		if (renderable)
		{
			NQVTK::TransferFunctionParamSet *tfps = 
				dynamic_cast<NQVTK::TransferFunctionParamSet*>(
					renderable->GetParamSet("transferfunction"));
			if (tfps)
			{
				tfps->tfStart = static_cast<float>(val) / 255.0;
				// TODO: use queued signal to prevent multiple separate updates
				mainView->updateGL();
			}
		}
	}

	void tfEndSlider_valueChanged(int val)
	{
		if (renderable)
		{
			NQVTK::TransferFunctionParamSet *tfps = 
				dynamic_cast<NQVTK::TransferFunctionParamSet*>(
					renderable->GetParamSet("transferfunction"));
			if (tfps)
			{
				tfps->tfEnd = static_cast<float>(val) / 255.0;
				// TODO: use queued signal to prevent multiple separate updates
				mainView->updateGL();
			}
		}
	}

	void pcaSlider_valueChanged(float val)
	{
		int modeId = sender()->property("modeId").toInt();
		if (renderable)
		{
			NQVTK::PCAParamSet *pcaps = 
				dynamic_cast<NQVTK::PCAParamSet*>(
					renderable->GetParamSet("pca"));
			if (pcaps)
			{
				pcaps->weights[modeId] = val;
				// TODO: use queued signal to prevent multiple separate updates
				mainView->updateGL();
			}
		}
	}
};
