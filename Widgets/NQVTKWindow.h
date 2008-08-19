#pragma once

#include <QMainWindow>

#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QString>
#include <QVBoxLayout>

#include "ui_NQVTKWindow.h"

#include "Rendering/SimpleRenderer.h"
#include "Rendering/LayeredRenderer.h"
#include "Rendering/CrossEyedStereoRenderer.h"
#include "Rendering/ShadowMappingRenderer.h"
#include "Rendering/BrushingRenderer.h"
#include "Rendering/OverlayRenderer.h"

#include "Rendering/PointFilteringRenderer.h"
#include "Rendering/PointCloud.h"

#include "Styles/DepthPeeling.h"
#include "Styles/IBIS.h"
#include "Styles/DistanceFields.h"

#include "Rendering/OrbitCamera.h"

#include "Rendering/PolyData.h"
#include "Rendering/ImageDataTexture3D.h"

#include "Rendering/DistanceFieldParamSet.h"
#include "Rendering/PCAParamSet.h"

#include "Interactors/MainViewInteractor.h"
#include "Interactors/BrushingInteractor.h"

#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataReader.h>

#include <vtkCylinderSource.h>
#include <vtkTriangleFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>

class NQVTKWindow : public QMainWindow
{
	Q_OBJECT;

public:
	NQVTKWindow(QWidget *parent = 0) : QMainWindow(parent) 
	{ 
		ui.setupUi(this);
	}

	void CreateRenderers()
	{
		// TODO: set up the renderers
		NQVTK::LayeredRenderer *renderer = 0;
		//if (!renderer) renderer = new NQVTK::LayeredRenderer();
		//if (!renderer) renderer = new NQVTK::CrossEyedStereoRenderer();
		if (!renderer) renderer = new NQVTK::ShadowMappingRenderer();

		ui.eyeSpacing->hide();
		ui.eyeSpacingLabel->hide();

		// Create the styles
		depthpeelStyle = new NQVTK::Styles::DepthPeeling();
		ibisStyle = new NQVTK::Styles::IBIS();
		distfieldStyle = new NQVTK::Styles::DistanceFields();

		// TODO: handle this somewhere else
		ibisStyle->SetOption("NQVTK_USE_PCA", "8");
		distfieldStyle->SetOption("NQVTK_USE_PCA", "8");

		// Set renderer style
		renderer->SetStyle(distfieldStyle);

		// Set camera to the interactive orbiting camera
		renderer->SetCamera(new NQVTK::OrbitCamera());

		NQVTK::BrushingRenderer *brushRen = new NQVTK::BrushingRenderer();

		//ui.nqvtkwidget->SetRenderer(renderer);
		ui.nqvtkwidget->SetRenderer(new NQVTK::OverlayRenderer(renderer, brushRen));
	}

	void LoadData()
	{
		// Renderables should be created in the right GL context
		ui.nqvtkwidget->makeCurrent();

		QHBoxLayout *layout = new QHBoxLayout(ui.simpleViewFrame);
		layout->setMargin(0);

		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		if (!renderer) return;

		qDebug("Creating and adding renderables...");
		QStringList args = qApp->arguments();
		if (args.size() < 2) 
		{
			qDebug("No arguments supplied, using default data...");
			// Set default testing data (on Vliet)
			/* - msdata
			args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2-textured.vtp");
			args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_08_02-T2-textured.vtp");
			args.append("-"); // distance field marker
			// NOTE: fields are flipped because object 1 should use distfield 0
			args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_08_02-T2-dist256.vti");
			args.append("D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2-dist256.vti");
			//*/
			/*  Cartilage
			args.append("D:/Data/Cartilage/Lorena/femoral2-textured.vtp");
			args.append("D:/Data/Cartilage/Lorena/femoral1-textured.vtp");
			args.append("-");
			args.append("D:/Data/Cartilage/Lorena/femoral1-dist256.vti");
			args.append("D:/Data/Cartilage/Lorena/femoral2-dist256.vti");
			//*/
			/* Ventricles
			args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20040510_textured.vtp");
			args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20041124_textured.vtp");
			args.append("-"); // distance field marker
			args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20041124_padded.vti");
			args.append("D:/Data/msdata/Ventricles/stef_ventricle_199_20040510_padded.vti");
			//*/
			/* Test heightfield
			args.append("D:/Data/Surface/test3-small-tri.vtp");
			args.append("D:/Data/Surface/test4-small-tri.vtp");
			args.append("-");
			args.append("D:/Data/Surface/test4-small-dist256.vti");
			args.append("D:/Data/Surface/test3-small-dist256.vti");
			//*/
			/* Luca's ventricles
			args.append("D:/data/Luca/PolyDataG0/Patient00-textured.vtp");
			args.append("D:/data/Luca/PolyDataG3/Patient00-textured.vtp");
			args.append("-");
			args.append("D:/data/Luca/PolyDataG3/Patient00-dist256.vti");
			args.append("D:/data/Luca/PolyDataG0/Patient00-dist256.vti");
			//*/
			//* Ventricle PCA
			args.append("D:/data/Luca/PCA/G0/mean-textured.vtp");
			args.append("D:/data/Luca/PCA/G3/mean-textured.vtp");
			//args.append("-");
			//args.append("D:/data/Luca/PCA/G3/mean-dist256.vti");
			//args.append("D:/data/Luca/PCA/G0/mean-dist256.vti");
			//*/
		}
		// Load the polydata
		NQVTK::Vector3 colors[] = {
			NQVTK::Vector3(1.0, 0.9, 0.4), 
			NQVTK::Vector3(0.3, 0.6, 1.0)
		};
		int i = 0;
		bool distFields = false;
		for (QStringList::const_iterator it = args.begin() + 1; it != args.end(); ++it)
		{
			qDebug("Loading %s #%d...", (distFields ? "distance field" : "surface"), i);
			if (QString("-") == *it)
			{
				distFields = true;
				i = 0;
				continue;
			}
			if (!distFields)
			{
				// Load surface
				vtkSmartPointer<vtkXMLPolyDataReader> reader = 
					vtkSmartPointer<vtkXMLPolyDataReader>::New();
				reader->SetFileName(it->toUtf8());
				reader->Update();
				NQVTK::Renderable *renderable = new NQVTK::PolyData(reader->GetOutput());
				renderable->color = colors[std::min(i, 1)];
				renderable->opacity = 0.3;
				renderer->AddRenderable(renderable);

				// Create a simple view showing just this renderable
				NQVTKWidget *simpleView = new NQVTKWidget(ui.simpleViewFrame, ui.nqvtkwidget);
				NQVTK::SimpleRenderer *simpleRen = new NQVTK::SimpleRenderer();
				simpleView->SetRenderer(simpleRen);
				connect(ui.nqvtkwidget, SIGNAL(cursorPosChanged(double, double)), 
					simpleView, SLOT(setCrosshairPos(double, double)));
				connect(ui.nqvtkwidget, SIGNAL(cameraUpdated(NQVTK::Camera*)), 
					simpleView, SLOT(syncCamera(NQVTK::Camera*)));
				connect(simpleView, SIGNAL(cameraUpdated(NQVTK::Camera*)), 
					ui.nqvtkwidget, SLOT(updateGL()));
				simpleView->toggleCrosshair(true);
				simpleView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
				layout->addWidget(simpleView);
				// TODO: sync camera after all widgets are initialized
				if (!simpleView->isSharing())
				{
					qDebug("WARNING! NQVTKWidgets can't share GL resources!");
					// TODO: we need to manually sync transformations between these objects
					simpleView->makeCurrent();
					NQVTK::Renderable *obj2 = new NQVTK::PolyData(reader->GetOutput()); 
					obj2->color = colors[std::min(i, 1)];
					simpleRen->AddRenderable(obj2);
				}
				else
				{
					// GL resources are shared, just add the original renderable
					simpleRen->AddRenderable(renderable);
				}
				simpleView->SetInteractor(new NQVTK::MainViewInteractor(simpleRen));
				ui.nqvtkwidget->makeCurrent();
			}
			else
			{
				// Assign to the renderable
				NQVTK::Renderable *renderable = renderer->GetRenderable(i);
				if (!renderable)
				{
					qDebug("Error! Found distance field for unknown renderable %d.", i);
				}
				else
				{
					// Load distance field
					vtkSmartPointer<vtkXMLImageDataReader> reader = 
						vtkSmartPointer<vtkXMLImageDataReader>::New();
					reader->SetFileName(it->toUtf8());
					reader->Update();
					NQVTK::ImageDataTexture3D *tex = 
						NQVTK::ImageDataTexture3D::New(reader->GetOutput());
					assert(tex);
					NQVTK::DistanceFieldParamSet *dfps = 
						new NQVTK::DistanceFieldParamSet(tex);
					renderable->SetParamSet("distancefield", dfps);
				}
			}
			++i;
		}

		//layout->addStretch();
		ui.simpleViewFrame->setLayout(layout);

		// Add a clipping cylinder for testing
		// TODO: make clipper object id adaptive to number of renderables
		{
			// Create a cylinder
			vtkSmartPointer<vtkCylinderSource> source = 
				vtkSmartPointer<vtkCylinderSource>::New();
			source->SetResolution(40);
			source->SetHeight(100);
			source->SetRadius(10);
			source->CappingOn();
			// Rotate the cylinder to the Z axis (otherwise we can't rotate it properly)
			vtkSmartPointer<vtkTransformPolyDataFilter> transformer = 
				vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			vtkSmartPointer<vtkTransform> transform = 
				vtkSmartPointer<vtkTransform>::New();
			transform->RotateX(90.0);
			transformer->SetInputConnection(source->GetOutputPort());
			transformer->SetTransform(transform);
			// NOTE: we need to triangulate, because NQVTK::PolyData currently only supports tris
			vtkSmartPointer<vtkTriangleFilter> triangulate = 
				vtkSmartPointer<vtkTriangleFilter>::New();
			triangulate->SetInputConnection(transformer->GetOutputPort());
			triangulate->Update();
			// Create the renderable
			NQVTK::Renderable *renderable = new NQVTK::PolyData(triangulate->GetOutput());
			renderer->AddRenderable(renderable);
			renderable->position = renderer->GetRenderable(0)->GetCenter();
			// Initially invisible
			renderable->visible = false;
			// For display in styles that don't support clipping
			renderable->opacity = 0.3;
			renderable->color = NQVTK::Vector3(1.0, 0.0, 0.0);
		}

		/* Add a brushing test widget
		{
			NQVTKWidget *brushWidget = new NQVTKWidget(ui.simpleViewFrame, ui.nqvtkwidget);
			layout->addWidget(brushWidget);
			NQVTK::BrushingRenderer *brushRen = new NQVTK::BrushingRenderer();
			brushWidget->SetRenderer(brushRen);
			NQVTK::BrushingInteractor *brushInt = new NQVTK::BrushingInteractor(brushRen);
			brushWidget->SetInteractor(brushInt);
			ui.nqvtkwidget->makeCurrent();
		}
		//*/

		//* Add PCA sliders
		// TODO: make this more flexible
		// TODO: also deform objects in simpleViews
		// TODO: should be able to manipulate objects separately
		// TODO: should be able to flip slider ranges
		// TODO: histograms above the sliders?
		QWidget *pcaWidget = new QWidget(ui.simpleViewFrame);
		pcaWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		QVBoxLayout *pcaLayout = new QVBoxLayout(pcaWidget);
		pcaLayout->setSpacing(0);
		pcaWidget->setLayout(pcaLayout);
		pcaWidget->setStyleSheet(
			"QSlider::groove:horizontal {"
			"	border: 1px solid #999999;"
			"	height: 1px;"
			"	background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
			"	margin: 3px 0;"
			"}"
			"QSlider::handle:horizontal {"
			"	border: 1px solid #5c5c5c;"
			"	width: 8px;"
			"	margin: -2px 0;"
			"}");
		const unsigned int numEigenModes = 8;
		for (unsigned int i = 0; i < numEigenModes; ++i)
		{
			QSlider *sld1 = new QSlider(pcaWidget);
			sld1->setRange(-300, 300);
			sld1->setValue(0);
			sld1->setOrientation(Qt::Horizontal);
			sld1->setProperty("modeId", static_cast<int>(i));
			sld1->setProperty("objectId", static_cast<int>(0));
			sld1->setStyleSheet("QSlider::handle { background: rgb(255, 230, 102); }");
			sld1->setMaximumHeight(5);
			connect(sld1, SIGNAL(valueChanged(int)), this, SLOT(pcaSlider_valueChanged(int)));
			QSlider *sld2 = new QSlider(pcaWidget);
			sld2->setRange(-300, 300);
			sld2->setValue(0);
			sld2->setOrientation(Qt::Horizontal);
			sld2->setProperty("modeId", static_cast<int>(i));
			sld2->setProperty("objectId", static_cast<int>(1));
			sld2->setStyleSheet("QSlider::handle { background: rgb(77, 153, 255); }");
			sld2->setMaximumHeight(5);
			connect(sld2, SIGNAL(valueChanged(int)), this, SLOT(pcaSlider_valueChanged(int)));
			QSlider *sld = new QSlider(pcaWidget);
			sld->setRange(-300, 300);
			sld->setValue(0);
			sld->setOrientation(Qt::Horizontal);
			sld->setStyleSheet("QSlider::handle { background: rgb(170, 170, 170); }");
			sld->setMaximumHeight(5);
			connect(sld, SIGNAL(valueChanged(int)), sld1, SLOT(setValue(int)));
			connect(sld, SIGNAL(valueChanged(int)), sld2, SLOT(setValue(int)));
			pcaLayout->addWidget(sld1);
			pcaLayout->addWidget(sld);
			pcaLayout->addWidget(sld2);
			pcaLayout->addSpacing(6);
		}
		// Give each renderable a PCAParamSet
		for (int i = 0; i < renderer->GetNumberOfRenderables(); ++i)
		{
			NQVTK::Renderable *renderable = renderer->GetRenderable(i);
			renderable->SetParamSet("pca", new NQVTK::PCAParamSet(numEigenModes));
		}
		layout->addWidget(pcaWidget);
		//*/

		// Set main view interactor
		// NOTE: This requires the camera and renderables to be set first
		NQVTK::MainViewInteractor *mainInt = new NQVTK::MainViewInteractor(
			ui.nqvtkwidget->GetRenderer(false));
		ui.nqvtkwidget->SetInteractor(mainInt);

		//ui.nqvtkwidget->StartContinuousUpdate();
	}

private:
	Ui::NQVTKWindow ui;

	NQVTK::Styles::DepthPeeling *depthpeelStyle;
	NQVTK::Styles::IBIS *ibisStyle;
	NQVTK::Styles::DistanceFields *distfieldStyle;

	void keyPressEvent(QKeyEvent *event)
	{
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		if (!renderer)
		{
			event->ignore();
			return;
		}

		// Quit on Escape
		switch (event->key())
		{
		case Qt::Key_Escape:
			qApp->quit();
			break;
		case Qt::Key_S:
			{
				QDateTime now = QDateTime::currentDateTime();
				QImage screenshot = ui.nqvtkwidget->grabFrameBuffer(true);
				// Fix alpha values
				screenshot.invertPixels(QImage::InvertRgba);
				screenshot.invertPixels(QImage::InvertRgb);
				// Save it
				screenshot.save(QString("NQVTK-%1.png").arg(
					now.toString("yyMMdd-hhmmss")), "PNG");
				break;
			}
		case Qt::Key_R:
			{
				renderer->ResetRenderables();
				// Reset clipper position
				NQVTK::Renderable *clipper = renderer->GetRenderable(2);
				if (clipper)
				{
					clipper->position = renderer->GetRenderable(0)->GetCenter();
				}
			}
			break;
		case Qt::Key_C:
			{
				// Toggle clipper visibility
				NQVTK::Renderable *clipper = renderer->GetRenderable(2);
				if (clipper)
				{
					clipper->visible = !clipper->visible;
				}
			}
			break;
		case Qt::Key_V:
			{
				// Viewpoint for the heightfield data, as used in the paper
				NQVTK::OrbitCamera *cam = 
					dynamic_cast<NQVTK::OrbitCamera*>(renderer->GetCamera());
				if (cam)
				{
					cam->zoom = 0.62;
					cam->rotateX = 29.0;
					cam->rotateY = 180.0;
				}
			}
			break;
		case Qt::Key_1:
			{
				NQVTK::Renderable *ren = renderer->GetRenderable(0);
				if (ren) ren->visible = !ren->visible;
			}
			break;
		case Qt::Key_2:
			{
				NQVTK::Renderable *ren = renderer->GetRenderable(1);
				if (ren) ren->visible = !ren->visible;
			}
			break;
		case Qt::Key_F1:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				// TODO: should disable the widget if this fails
				if (lrenderer) lrenderer->SetStyle(depthpeelStyle);
			}
			break;
		case Qt::Key_F2:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				// TODO: should disable the widget if this fails
				if (lrenderer) lrenderer->SetStyle(ibisStyle);
			}
			break;
		case Qt::Key_F3:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				// TODO: should disable the widget if this fails
				if (lrenderer) lrenderer->SetStyle(distfieldStyle);
			}
			break;
		default:
			event->ignore();
			return;
		}
		event->accept();

		ui.nqvtkwidget->updateGL();
		// TODO: also update all simple views
	}

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
		distfieldStyle->useGridTexture = val;
		ui.nqvtkwidget->updateGL();
	}

	void on_useGlyphTexture_toggled(bool val) 
	{
		distfieldStyle->useGlyphTexture = val;
		ui.nqvtkwidget->updateGL();
	}

	void on_classificationThreshold_valueChanged(int val) 
	{
		distfieldStyle->classificationThreshold = 
			static_cast<double>(val) / 100.0;
		ui.nqvtkwidget->updateGL();
	}

	void on_useDistanceColorMap_toggled(bool val) 
	{
		distfieldStyle->useDistanceColorMap = val;
		ui.nqvtkwidget->updateGL();
	}

	void on_contourDepthEpsilon_valueChanged(int val) 
	{
		distfieldStyle->contourDepthEpsilon = 
			static_cast<double>(val) / 1000.0;
		ui.nqvtkwidget->updateGL();
	}

	void on_useFatContours_toggled(bool val) 
	{ 
		distfieldStyle->useFatContours = val;
		ui.nqvtkwidget->updateGL();
	}

	void on_depthCueRange_valueChanged(int val) 
	{
		distfieldStyle->depthCueRange = 
			static_cast<double>(val) / 1000.0;
		ui.nqvtkwidget->updateGL();
	}

	void on_useFog_toggled(bool val) 
	{
		distfieldStyle->useFog = val;
		ui.nqvtkwidget->updateGL();
	}

	void on_opacity_valueChanged(int val)
	{
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		for (int i = 0; i < renderer->GetNumberOfRenderables(); ++i)
		{
			renderer->GetRenderable(i)->opacity = 
				static_cast<double>(val) / 100.0;
		}
		ui.nqvtkwidget->updateGL();
	}

	void on_maxLayers_valueChanged(int val)
	{
		NQVTK::LayeredRenderer *renderer = 
			dynamic_cast<NQVTK::LayeredRenderer*>(ui.nqvtkwidget->GetRenderer());
		if (renderer)
		{
			renderer->maxLayers = val;
			ui.nqvtkwidget->updateGL();
		}
	}

	void on_eyeSpacing_valueChanged(int val)
	{
		// Do we have a stereo renderer?
		NQVTK::CrossEyedStereoRenderer *renderer = 
			dynamic_cast<NQVTK::CrossEyedStereoRenderer*>(ui.nqvtkwidget->GetRenderer());
		if (renderer)
		{
			renderer->eyeSpacing = 
				static_cast<double>(val) / 1000.0;
		}
	}

	void on_lightOffsetDirection_valueChanged(int val)
	{
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		renderer->lightOffsetDirection = val;
		// TODO: also set this option for all SimpleRenderers?
		ui.nqvtkwidget->updateGL();
	}

	void on_lightRelativeToCamera_toggled(bool val)
	{
		ui.lightOffsetDirection->setEnabled(val);
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		renderer->lightRelativeToCamera = val;
		// TODO: also set this option for all SimpleRenderers?
		ui.nqvtkwidget->updateGL();
	}

	void pcaSlider_valueChanged(int val)
	{
		int modeId = sender()->property("modeId").toInt();
		int objectId = sender()->property("objectId").toInt();
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		NQVTK::PCAParamSet *pcaParams = dynamic_cast<NQVTK::PCAParamSet*>(
			renderer->GetRenderable(objectId)->GetParamSet("pca"));
		if (pcaParams)
		{
			pcaParams->weights[modeId] = static_cast<float>(val) / 100.0;
			ui.nqvtkwidget->updateGL();
		}
	}

	void on_brushTest_clicked()
	{
		// Set up the filter
		NQVTK::PointFilteringRenderer *filter = new NQVTK::PointFilteringRenderer();
		NQVTK::Renderer *ren = ui.nqvtkwidget->GetRenderer();
		filter->SetCamera(ren->GetCamera());
		filter->Initialize();
		filter->Resize(ren->GetWidth(), ren->GetHeight());
		// Add the object
		NQVTK::Renderable *renderable = ren->GetRenderable(0);
		filter->AddRenderable(renderable);
		// Set the mask
		NQVTK::OverlayRenderer *oren = 
			dynamic_cast<NQVTK::OverlayRenderer*>(
				ui.nqvtkwidget->GetRenderer(false));
		if (oren)
		{
			NQVTK::Renderer *bren = oren->GetOverlayRenderer();
			if (bren)
			{
				GLFramebuffer *target = bren->GetTarget();
				if (target)
				{
					filter->SetMask(target->GetTexture2D());
				}
			}
		}
		// Perform filtering render
		filter->Draw();
		// TODO: Get results
		//NQVTK::PointCloud *pc = new NQVTK::PointCloud(renderable, filter->pointIds);
		//ren->AddRenderable(pc);
		// Clean up
		delete filter;
	}
};
