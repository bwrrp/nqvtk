#pragma once

#include <QMainWindow>

#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QHostInfo>
#include <QKeyEvent>
#include <QString>
#include <QVBoxLayout>

#include "ui_NQVTKWindow.h"

#include "RenderableControlWidget.h"

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
#include "Styles/Raycaster.h"

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

		// NOTE: Adding a layout breaks stretchfactors (bug?)
		// Therefore we restore the old size after adding the layout
		QSize framesize = ui.simpleViewFrame->size();
		QHBoxLayout *layout = new QHBoxLayout(ui.simpleViewFrame);
		layout->setMargin(0);
		ui.simpleViewFrame->resize(framesize);
	}

	void SetupViews()
	{
		// Set up the main view
		NQVTK::LayeredRenderer *renderer;
		renderer = new NQVTK::LayeredRenderer();
		//renderer = new NQVTK::CrossEyedStereoRenderer();
		//renderer = new NQVTK::ShadowMappingRenderer();

		// Hide eye spacing widgets (only works for stereo renderers)
		ui.eyeSpacing->hide();
		ui.eyeSpacingLabel->hide();
		// This isn't implemented right now (should be a new style)
		ui.useDistanceColorMap->hide();
		// Hide advanced widgets
		ui.contourDepthEpsilon->hide();
		ui.contourDepthEpsilonLabel->hide();
		ui.brushTest->hide();

		// Create the styles
		depthpeelStyle = new NQVTK::Styles::DepthPeeling();
		ibisStyle = new NQVTK::Styles::IBIS();
		distfieldStyle = new NQVTK::Styles::DistanceFields();

		// TODO: handle this somewhere else
		ibisStyle->SetOption("NQVTK_USE_PCA", "8");
		distfieldStyle->SetOption("NQVTK_USE_PCA", "8");

		// Set renderer style
		//renderer->SetStyle(distfieldStyle);
		renderer->SetStyle(new NQVTK::Styles::Raycaster());

		// Set camera to the interactive orbiting camera
		renderer->SetCamera(new NQVTK::OrbitCamera());

		// Add a brushing overlay
		NQVTK::BrushingRenderer *brushRen = new NQVTK::BrushingRenderer();
		ui.nqvtkwidget->SetRenderer(new NQVTK::OverlayRenderer(renderer, brushRen));
	}

	void LoadData()
	{
		// Renderables should be created in the right GL context
		ui.nqvtkwidget->makeCurrent();

		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		if (!renderer) return;

		qDebug("Parsing arguments...");
		QStringList args = qApp->arguments();
		int numObjects = 2;
		std::vector<std::string> meshPaths;
		std::vector<std::string> distFieldPaths;
		if (args.size() < 2) 
		{
			// Are we running on Vliet?
			QString hostname = QHostInfo::localHostName();
			if (hostname.toLower() == QString("vliet"))
			{
				qDebug("Running on Vliet, using default data");
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
				/* Ventricle PCA
				args.append("D:/data/Luca/PCA/G0/mean-textured.vtp");
				args.append("D:/data/Luca/PCA/G3/mean-textured.vtp");
				//args.append("-");
				//args.append("D:/data/Luca/PCA/G3/mean-dist256.vti");
				//args.append("D:/data/Luca/PCA/G0/mean-dist256.vti");
				//*/
				//* Raycaster test
				args.append("D:/Data/Misc/stent8_256_box.vtp");
				args.append("-");
				args.append("D:/Data/Misc/stent8_256.vti");
				//*/
			}
			else
			{
				qDebug("No arguments supplied");
			}
		}

		if (args.size() >= 2)
		{
			// Get paths from arguments
			bool distFields = false;
			for (QStringList::const_iterator it = args.begin() + 1; 
				it != args.end(); ++it)
			{
				if (QString("-") == *it)
				{
					distFields = true;
					continue;
				}
				if (!distFields)
				{
					meshPaths.push_back(std::string(it->toUtf8()));
				}
				else
				{
					distFieldPaths.push_back(std::string(it->toUtf8()));
				}
			}
			if (distFieldPaths.size() > meshPaths.size())
			{
				qDebug("Error! Number of distance fields is larger than number of objects!");
				return;
			}
			numObjects = meshPaths.size();
		}

		qDebug("Loading %d surfaces and %d distance fields...", 
			meshPaths.size(), distFieldPaths.size());

		// Setup secondary views
		std::vector<NQVTK::Vector3> colors;
		colors.push_back(NQVTK::Vector3(1.0, 0.9, 0.4));
		colors.push_back(NQVTK::Vector3(0.3, 0.6, 1.0));
		colors.push_back(NQVTK::Vector3(1.0, 1.0, 1.0));
		int maxColor = colors.size() - 1;
		for (int i = 0; i < numObjects; ++i)
		{
			RenderableControlWidget *rcw = 
				new RenderableControlWidget(ui.nqvtkwidget);
			rcw->SetColor(colors[std::min(i, maxColor)]);
			rcw->SetOpacity(static_cast<double>(ui.opacity->value()) / 100.0);
			renderableControlWidgets.push_back(rcw);
			AddTrayWidget(rcw);
		}

		// Load the polydata
		for (unsigned int i = 0; i < meshPaths.size(); ++i)
		{
			qDebug("Loading surface #%d...", i);
			renderableControlWidgets[i]->LoadMesh(meshPaths[i]);
		}

		// Load the distance fields
		for (unsigned int i = 0; i < distFieldPaths.size(); ++i)
		{
			qDebug("Loading distance field #%d...", i);
			renderableControlWidgets[i]->LoadDistanceField(distFieldPaths[i]);
		}

		// Add a clipping cylinder for testing
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
			// TODO: make clipper object id adaptive to number of renderables
			// Alternatively: make every object capable of being a clipper
			int id = renderer->AddRenderable(renderable);
			assert(id = 2);
			if (renderer->GetRenderable(0))
			{
				// TODO: adapt position to new renderables
				renderable->position = renderer->GetRenderable(0)->GetCenter();
			}
			// Initially invisible
			renderable->visible = false;
			// For display in styles that don't support clipping
			renderable->opacity = 0.3;
			renderable->color = NQVTK::Vector3(1.0, 0.0, 0.0);
		}

		// Set main view interactor
		// NOTE: This requires the camera and renderables to be set first
		// TODO: add a way to update interactors
		NQVTK::MainViewInteractor *mainInt = new NQVTK::MainViewInteractor(
			ui.nqvtkwidget->GetRenderer(false));
		ui.nqvtkwidget->SetInteractor(mainInt);

		//ui.nqvtkwidget->StartContinuousUpdate();
	}

	void AddTrayWidget(QWidget *widget)
	{
		widget->setParent(ui.simpleViewFrame);
		widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		QLayout *layout = ui.simpleViewFrame->layout();
		assert(layout);
		layout->addWidget(widget);
	}

protected:
	Ui::NQVTKWindow ui;

	NQVTK::Styles::DepthPeeling *depthpeelStyle;
	NQVTK::Styles::IBIS *ibisStyle;
	NQVTK::Styles::DistanceFields *distfieldStyle;

	std::vector<RenderableControlWidget*> renderableControlWidgets;

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
			static_cast<double>(val) / 1000.0;
		ui.classificationThresholdDisplay->setText(QString("%1").arg(
			distfieldStyle->classificationThreshold, 0, 'f', 3));
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
		for (std::vector<RenderableControlWidget*>::iterator it = 
			renderableControlWidgets.begin(); 
			it != renderableControlWidgets.end(); ++it)
		{
			(*it)->SetOpacity(static_cast<double>(val) / 100.0);
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
			GLTexture *mask = oren->GetOverlayImage();
			filter->SetMask(mask);
		}
		// Perform filtering render
		filter->Draw();
		// Get the results
		if (filter->pointIds.size() > 0)
		{
			NQVTK::VBOMesh *mesh = dynamic_cast<NQVTK::VBOMesh*>(renderable);
			if (mesh)
			{
				NQVTK::PointCloud *pc = new NQVTK::PointCloud(mesh, filter->pointIds);
				pc->color = NQVTK::Vector3(0.3, 0.3, 0.3);
				// TODO: create a new view to further refine the selection
				// possibly as a second overlay on the main view
				NQVTKWidget *testWidget = new NQVTKWidget(0, ui.nqvtkwidget);
				NQVTK::Renderer *testRen = new NQVTK::SimpleRenderer();
				testWidget->SetRenderer(testRen);
				AddTrayWidget(testWidget);
				// Connect signals
				connect(ui.nqvtkwidget, SIGNAL(cursorPosChanged(double, double)), 
					testWidget, SLOT(setCrosshairPos(double, double)));
				connect(ui.nqvtkwidget, SIGNAL(cameraUpdated(NQVTK::Camera*)), 
					testWidget, SLOT(syncCamera(NQVTK::Camera*)));
				// Set params
				testWidget->toggleCrosshair(true);
				// Add brushing result
				testRen->AddRenderable(pc);
			}
		}
		// Clean up
		delete filter;
	}
};
