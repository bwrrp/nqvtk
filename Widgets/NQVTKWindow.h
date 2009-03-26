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
#include "Rendering/LayeredRaycastingRenderer.h"
#include "Rendering/CrossEyedStereoRenderer.h"
#include "Rendering/ShadowMappingRenderer.h"
#include "Rendering/BrushingRenderer.h"
#include "Rendering/OverlayRenderer.h"

#include "Rendering/PointFilteringRenderer.h"
#include "Rendering/PointCloud.h"

#include "Rendering/PCAPointCorrespondenceGlyphs.h"

#include "Styles/DepthPeeling.h"
#include "Styles/IBIS.h"
#include "Styles/DistanceFields.h"
#include "Styles/Raycaster.h"
#include "Styles/DeformationRaycaster.h"
#include "Styles/LayeredRaycaster.h"

#include "Rendering/ArcballCamera.h"
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
		//renderer = new NQVTK::LayeredRenderer();
		renderer = new NQVTK::LayeredRaycastingRenderer();
		// TODO: stereo renderer should use a nested renderer, not inheritance
		// TODO: shadowmapping renderer should use a nested renderer, not inheritance
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
		raycastStyle = new NQVTK::Styles::Raycaster();
		deformationStyle = new NQVTK::Styles::DeformationRaycaster();
		layeredRaycastStyle = new NQVTK::Styles::LayeredRaycaster();

		// TODO: handle this somewhere else
		ibisStyle->SetOption("NQVTK_USE_PCA", "8");
		distfieldStyle->SetOption("NQVTK_USE_PCA", "8");

		// Set renderer style
		//renderer->SetStyle(distfieldStyle);
		//renderer->SetStyle(raycastStyle);
		//renderer->SetStyle(deformationStyle);
		renderer->SetStyle(layeredRaycastStyle);

		// Set initial widget group visibility
		ui.ibisGroup->hide();
		ui.scalarsGroup->hide();
		ui.raycasterGroup->show();
		ui.deformationGroup->hide();

		// Set initial camera
		//renderer->SetCamera(new NQVTK::OrbitCamera());
		renderer->SetCamera(new NQVTK::ArcballCamera());

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
				args.append("D:/data/Luca/PCA/G0/mean-textured.vtp");
				args.append("-");
				args.append("D:/data/Luca/PCA/G0/mean-dist256.vti");
				args.append("D:/data/Luca/PCA/G0/mean-dist256.vti");
				//*/
				/* Raycaster test
				args.append("D:/Data/Misc/stent8_256_box.vtp");
				args.append("D:/Data/Misc/stent8_256_box.vtp");
				args.append("-");
				args.append("D:/Data/Misc/stent8_256.vti");
				args.append("D:/Data/Misc/stent8_256.vti");
				//*/
				/* Raycaster test 2
				args.append("D:/Data/Cartilage/cartilage3D/new_data_20060812/vti/PatientA-Sagittal1_box.vtp");
				args.append("D:/Data/Cartilage/cartilage3D/new_data_20060812/vti/PatientA-Transversal_box.vtp");
				args.append("D:/Data/Cartilage/cartilage3D/new_data_20060812/vti/PatientA-Sagittal2_box.vtp");
				args.append("-");
				args.append("D:/Data/Cartilage/cartilage3D/new_data_20060812/vti/PatientA-Sagittal1.vti");
				args.append("D:/Data/Cartilage/cartilage3D/new_data_20060812/vti/PatientA-Transversal.vti");
				args.append("D:/Data/Cartilage/cartilage3D/new_data_20060812/vti/PatientA-Sagittal2.vti");
				//*/
				/* msdata lesion masks
				args.append("D:/Data/msdata/T2W/T2W_lesionmasks_normalized_16its/padded/wn0920-TP_2005.10.21-Lesions(DUO)-mask_LES_box.vtp");
				args.append("D:/Data/msdata/T2W/T2W_lesionmasks_normalized_16its/padded/wn0920-TP_2006.03.02-Lesions(DUO)-mask_LES_box.vtp");
				args.append("-");
				args.append("D:/Data/msdata/T2W/T2W_lesionmasks_normalized_16its/padded/wn0920-TP_2005.10.21-Lesions(DUO)-mask_LES.mha");
				args.append("D:/Data/msdata/T2W/T2W_lesionmasks_normalized_16its/padded/wn0920-TP_2006.03.02-Lesions(DUO)-mask_LES.mha");
				//*/
				/* Deformation fields - msdata
				//args.append("D:/Data/msdata/T2W/T2W_images_normalized/wn0920-F051114-M060203/deformationField_box.vtp");
				args.append("D:/Data/msdata/T2W/T2W_images_normalized/wn0920-F051114-M060302-hr/deformationField_box.vtp");
				args.append("D:/Data/msdata/T2W/T2W_images_normalized/wn0920-TP_2005_11_14-T2_box.vtp");
				args.append("-");
				//args.append("D:/Data/msdata/T2W/T2W_images_normalized/wn0920-F051114-M060203/padded/deformationField.mha");
				args.append("D:/Data/msdata/T2W/T2W_images_normalized/wn0920-F051114-M060302-hr/padded/deformationField.mha");
				args.append("D:/Data/msdata/T2W/T2W_images_normalized/padded/wn0920-TP_2005_11_14-T2.mha");
				//*/
				/* Deformation fields - knee cartilage
				args.append("D:/Data/Cartilage/cartilage3D/elastix-190x-nonrigid/deformationField_box.vtp");
				args.append("D:/Data/Cartilage/cartilage3D/GARP190-1.2-s401-masked_box.vtp");
				//args.append("D:/Data/Cartilage/cartilage3D/elastix-190x-nonrigid/result_box.vtp");
				args.append("-");
				args.append("D:/Data/Cartilage/cartilage3D/elastix-190x-nonrigid/deformationField.mhd");
				args.append("D:/Data/Cartilage/cartilage3D/GARP190-1.2-s401-masked.mha");
				//args.append("D:/Data/Cartilage/cartilage3D/elastix-190x-nonrigid/result.mhd");
				//*/
				/* Deformation fields - ms data with artificial deformation
				args.append("D:/Temp/n0231-TP_2004_06_28-T1_3D_meep_box.vtp");
				args.append("D:/Data/msdata/T1W/T1_3D/T1_3D_images_original/n0231-TP_2004_06_28-T1_3D_box.vtp");
				//args.append("D:/Data/msdata/T1W/T1_3D/T1_3D_images_original/n0221-TP_2004_12_06-T1_3D_box.vtp");
				args.append("-");
				args.append("D:/Temp/n0231-TP_2004_06_28-T1_3D_meep.mha");
				args.append("D:/Data/msdata/T1W/T1_3D/T1_3D_images_original/n0231-TP_2004_06_28-T1_3D.mha");
				//args.append("D:/Data/msdata/T1W/T1_3D/T1_3D_images_original/n0221-TP_2004_12_06-T1_3D.mha");
				//*/
				//* Deformation fields - test
				args.append("D:/Data/VectorFields/meep_box.vtp");
				args.append("D:/Data/VectorFields/meep_box.vtp");
				args.append("-");
				args.append("D:/Data/VectorFields/meep2.mha");
				args.append("D:/Data/VectorFields/meep2.mha");
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

		qDebug("Loading %d surfaces and %d volumes...", 
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

		/* Add a clipping cylinder for testing
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
			// TODO: make every object capable of being a clipper
			int id = renderer->AddRenderable(renderable);
			ibisStyle->clipId = id;
			distfieldStyle->clipId = id;
			assert(id > 1);
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
		//*/

		/* Add renderable for point correspondence glyphs
		// TODO: update this when one of the meshes changes
		{
			NQVTK::VBOMesh *obj0 = dynamic_cast<NQVTK::VBOMesh*>(renderer->GetRenderable(0));
			NQVTK::VBOMesh *obj1 = dynamic_cast<NQVTK::VBOMesh*>(renderer->GetRenderable(1));
			assert(obj0);
			assert(obj1);
			NQVTK::Renderable *renderable = new NQVTK::PCAPointCorrespondenceGlyphs(obj0, obj1);
			renderer->AddRenderable(renderable);
			// Initially invisible
			renderable->visible = false;
			renderable->color = NQVTK::Vector3(0.0, 0.0, 0.0);
			renderable->opacity = 1.0;
		}
		//*/

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
	NQVTK::Styles::Raycaster *raycastStyle;
	NQVTK::Styles::DeformationRaycaster *deformationStyle;
	NQVTK::Styles::LayeredRaycaster *layeredRaycastStyle;

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
				NQVTK::Renderable *clipper = renderer->GetRenderable(ibisStyle->clipId);
				if (clipper && renderer->GetRenderable(0))
				{
					clipper->position = renderer->GetRenderable(0)->GetCenter();
				}
			}
			break;
		case Qt::Key_C:
			{
				// Toggle clipper visibility
				NQVTK::Renderable *clipper = renderer->GetRenderable(ibisStyle->clipId);
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
					// Landscape data
					//cam->zoom = 0.62;
					//cam->rotateX = 29.0;
					//cam->rotateY = 180.0;

					// Deformation test volume
					cam->zoom = 0.85;
					cam->rotateX = -35.0;
					cam->rotateY = 135.0;
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
		case Qt::Key_3:
			{
				NQVTK::Renderable *ren = renderer->GetRenderable(2);
				if (ren) ren->visible = !ren->visible;
			}
			break;
		case Qt::Key_4:
			{
				NQVTK::Renderable *ren = renderer->GetRenderable(3);
				if (ren) ren->visible = !ren->visible;
			}
			break;
		case Qt::Key_F1:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				if (lrenderer) 
				{
					lrenderer->SetStyle(depthpeelStyle);
					ui.ibisGroup->hide();
					ui.scalarsGroup->hide();
					ui.raycasterGroup->hide();
					ui.deformationGroup->hide();
				}
			}
			break;
		case Qt::Key_F2:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				if (lrenderer) 
				{
					lrenderer->SetStyle(ibisStyle);
					ui.ibisGroup->show();
					ui.scalarsGroup->show();
					ui.raycasterGroup->hide();
					ui.deformationGroup->hide();
				}
			}
			break;
		case Qt::Key_F3:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				if (lrenderer) 
				{
					lrenderer->SetStyle(distfieldStyle);
					ui.ibisGroup->show();
					ui.scalarsGroup->show();
					ui.raycasterGroup->hide();
					ui.deformationGroup->hide();
				}
			}
			break;
		case Qt::Key_F4:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				if (lrenderer) 
				{
					lrenderer->SetStyle(raycastStyle);
					ui.ibisGroup->hide();
					ui.scalarsGroup->hide();
					ui.raycasterGroup->show();
					ui.deformationGroup->hide();
				}
			}
			break;
		case Qt::Key_F5:
			{
				NQVTK::LayeredRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRenderer*>(renderer);
				if (lrenderer) 
				{
					lrenderer->SetStyle(deformationStyle);
					ui.ibisGroup->hide();
					ui.scalarsGroup->hide();
					ui.raycasterGroup->show();
					ui.deformationGroup->show();
				}
			}
			break;
		case Qt::Key_F6:
			{
				NQVTK::LayeredRaycastingRenderer *lrenderer = 
					dynamic_cast<NQVTK::LayeredRaycastingRenderer*>(renderer);
				if (lrenderer)
				{
					lrenderer->SetStyle(layeredRaycastStyle);
					ui.ibisGroup->hide();
					ui.scalarsGroup->hide();
					ui.raycasterGroup->show();
					ui.deformationGroup->hide();
				}
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

	// IBIS parameters
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
	void on_distanceThreshold_valueChanged(int val) 
	{
		distfieldStyle->distanceThreshold = 
			static_cast<double>(val) / 1000.0;
		ui.distanceThresholdDisplay->setText(
			QString::fromUtf8("Distance \342\211\245 %1").arg( // utf-8 >= character
				distfieldStyle->distanceThreshold, 0, 'f', 3));
		
		ui.nqvtkwidget->updateGL();
	}
	void on_pvalueThreshold_valueChanged(int val) 
	{
		ibisStyle->pvalueThreshold = 
			static_cast<double>(val) / 1000.0;
		distfieldStyle->pvalueThreshold = ibisStyle->pvalueThreshold;
		ui.pvalueThresholdDisplay->setText(
			QString::fromUtf8("P-value \342\211\244 %1").arg( // utf-8 <= character
				distfieldStyle->pvalueThreshold, 0, 'f', 3));
		
		ui.nqvtkwidget->updateGL();
	}
	void on_useDistanceColorMap_toggled(bool val) 
	{
		distfieldStyle->useDistanceColorMap = val;
		ui.nqvtkwidget->updateGL();
	}
	void on_contourDepthEpsilon_valueChanged(int val) 
	{
		ibisStyle->contourDepthEpsilon = 
			static_cast<double>(val) / 1000.0;
		distfieldStyle->contourDepthEpsilon = ibisStyle->contourDepthEpsilon;
		ui.nqvtkwidget->updateGL();
	}
	void on_useContours_toggled(bool val)
	{
		ibisStyle->useContours = val;
		distfieldStyle->useContours = val;
		ui.nqvtkwidget->updateGL();
	}
	void on_useFatContours_toggled(bool val) 
	{
		ibisStyle->useFatContours = val;
		distfieldStyle->useFatContours = val;
		ui.nqvtkwidget->updateGL();
	}
	void on_depthCueRange_valueChanged(int val) 
	{
		ibisStyle->depthCueRange = 
			static_cast<double>(val) / 1000.0;
		distfieldStyle->depthCueRange = ibisStyle->depthCueRange;
		ui.nqvtkwidget->updateGL();
	}
	void on_useFog_toggled(bool val) 
	{
		ibisStyle->useFog = val;
		distfieldStyle->useFog = val;
		ui.nqvtkwidget->updateGL();
	}

	// General parameters
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

	// Raycaster parameters
	void on_stepSize_valueChanged(int val)
	{
		raycastStyle->stepSize = static_cast<double>(val) / 10.0;
		deformationStyle->stepSize = raycastStyle->stepSize;
		layeredRaycastStyle->stepSize = raycastStyle->stepSize;
		ui.stepSize->setToolTip(QString("%1").arg(raycastStyle->stepSize));

		ui.nqvtkwidget->updateGL();
	}
	void on_kernelSize_valueChanged(int val)
	{
		raycastStyle->kernelSize = static_cast<double>(val) / 10.0;
		deformationStyle->kernelSize = raycastStyle->kernelSize;
		layeredRaycastStyle->kernelSize = raycastStyle->kernelSize;
		ui.kernelSize->setToolTip(QString("%1").arg(raycastStyle->kernelSize));

		ui.nqvtkwidget->updateGL();
	}

	// Test purposes only
	void on_testParam_valueChanged(int val)
	{
		// TODO: remove testParam later
		deformationStyle->testParam = static_cast<float>(val) / 100.0;
		layeredRaycastStyle->testParam = static_cast<float>(val) / 100.0;

		ui.nqvtkwidget->updateGL();
	}

	// Smear parameters
	void on_smearTFStart_valueChanged(int val)
	{
		deformationStyle->smearTFStart = static_cast<float>(val) / 100.0;
		ui.smearTFStart->setToolTip(QString("%1").arg(deformationStyle->smearTFStart));

		ui.nqvtkwidget->updateGL();
	}
	void on_smearTFEnd_valueChanged(int val)
	{
		deformationStyle->smearTFEnd = static_cast<float>(val) / 100.0;
		ui.smearTFEnd->setToolTip(QString("%1").arg(deformationStyle->smearTFEnd));

		ui.nqvtkwidget->updateGL();
	}
	void on_smearDensity_valueChanged(int val)
	{
		deformationStyle->smearDensity = static_cast<float>(val) / 100.0;
		ui.smearDensity->setToolTip(QString("%1").arg(deformationStyle->smearDensity));

		ui.nqvtkwidget->updateGL();
	}
	void on_smearLength_valueChanged(int val)
	{
		deformationStyle->smearLength = static_cast<float>(val) / 100.0;
		ui.smearLength->setToolTip(QString("%1").arg(deformationStyle->smearLength));

		ui.nqvtkwidget->updateGL();
	}

	// Interest functions
	void on_focusIFStart_valueChanged(int val)
	{
		deformationStyle->focusIFStart = static_cast<float>(val) / 100.0;
		ui.focusIFStart->setToolTip(QString("%1").arg(deformationStyle->focusIFStart));

		ui.nqvtkwidget->updateGL();
	}
	void on_focusIFEnd_valueChanged(int val)
	{
		deformationStyle->focusIFEnd = static_cast<float>(val) / 100.0;
		ui.focusIFEnd->setToolTip(QString("%1").arg(deformationStyle->focusIFEnd));

		ui.nqvtkwidget->updateGL();
	}
	void on_staticIFStart_valueChanged(int val)
	{
		deformationStyle->staticIFStart = static_cast<float>(val) / 100.0;
		ui.staticIFStart->setToolTip(QString("%1").arg(deformationStyle->staticIFStart));

		ui.nqvtkwidget->updateGL();
	}
	void on_staticIFEnd_valueChanged(int val)
	{
		deformationStyle->staticIFEnd = static_cast<float>(val) / 100.0;
		ui.staticIFEnd->setToolTip(QString("%1").arg(deformationStyle->staticIFEnd));

		ui.nqvtkwidget->updateGL();
	}
	void on_dynamicIFStart_valueChanged(int val)
	{
		deformationStyle->dynamicIFStart = static_cast<float>(val) / 100.0;
		ui.dynamicIFStart->setToolTip(QString("%1").arg(deformationStyle->dynamicIFStart));

		ui.nqvtkwidget->updateGL();
	}
	void on_dynamicIFEnd_valueChanged(int val)
	{
		deformationStyle->dynamicIFEnd = static_cast<float>(val) / 100.0;
		ui.dynamicIFEnd->setToolTip(QString("%1").arg(deformationStyle->dynamicIFEnd));

		ui.nqvtkwidget->updateGL();
	}

	// Lighting
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
