#pragma once

#include <QMainWindow>

#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QString>

#include "ui_NQVTKWindow.h"

#include "Rendering/SimpleRenderer.h"
#include "Rendering/LayeredRenderer.h"
#include "Rendering/CrossEyedStereoRenderer.h"
#include "Rendering/ShadowMappingRenderer.h"

#include "Styles/DepthPeeling.h"
#include "Styles/IBIS.h"
#include "Styles/DistanceFields.h"

#include "Rendering/OrbitCamera.h"

#include "Rendering/PolyData.h"
#include "Rendering/ImageDataTexture3D.h"

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

		// Create the styles
		depthpeelStyle = new NQVTK::Styles::DepthPeeling();
		ibisStyle = new NQVTK::Styles::IBIS();
		distfieldStyle = new NQVTK::Styles::DistanceFields();

		// Set renderer style
		renderer->SetStyle(distfieldStyle);
		//renderer->SetStyle(new NQVTK::Styles::ShadowMap());

		// Set camera to the interactive orbiting camera
		renderer->SetCamera(new NQVTK::OrbitCamera());

		ui.nqvtkwidget->SetRenderer(renderer);
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
			//* Ventricles
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

				// TESTING
				NQVTKWidget *simpleView = new NQVTKWidget(ui.simpleViewFrame);
				layout->addWidget(simpleView);
				NQVTK::SimpleRenderer *simpleRen = new NQVTK::SimpleRenderer();
				simpleView->SetRenderer(simpleRen);
				connect(ui.nqvtkwidget, SIGNAL(cursorPosChanged(double, double)), 
					simpleView, SLOT(setCrosshairPos(double, double)));
				connect(ui.nqvtkwidget, SIGNAL(cameraUpdated(NQVTK::Camera*)), 
					simpleView, SLOT(syncCamera(NQVTK::Camera*)));
				simpleView->toggleCrosshair(true);
				simpleView->resize(128,128);
				simpleView->show();
				simpleView->makeCurrent();
				NQVTK::Renderable *obj2 = new NQVTK::PolyData(reader->GetOutput()); 
				obj2->color = colors[std::min(i, 1)];
				simpleRen->AddRenderable(obj2);
				ui.nqvtkwidget->makeCurrent();
			}
			else
			{
				// Load distance field
				vtkSmartPointer<vtkXMLImageDataReader> reader = 
					vtkSmartPointer<vtkXMLImageDataReader>::New();
				reader->SetFileName(it->toUtf8());
				reader->Update();
				GLTexture *tex = NQVTK::ImageDataTexture3D::New(reader->GetOutput());
				assert(tex);
				distfieldStyle->SetDistanceField(i, tex);
			}
			++i;
		}

		//layout->addStretch();
		ui.simpleViewFrame->setLayout(layout);

		// Add a clipping cylinder for testing
		// TODO: add option to toggle clipper object on or off
		// TODO: add interaction to move / rotate the clipper object
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
			// NOTE: we need to triangulate this, because NQVTK::PolyData only supports tris
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
			// For display in non-clipping styles
			renderable->opacity = 0.3;
			renderable->color = NQVTK::Vector3(1.0, 0.0, 0.0);
		}
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
	}

	void on_useGlyphTexture_toggled(bool val) 
	{
		distfieldStyle->useGlyphTexture = val;
	}

	void on_classificationThreshold_valueChanged(int val) 
	{
		distfieldStyle->classificationThreshold = 
			static_cast<double>(val) / 100.0;
	}

	void on_useDistanceColorMap_toggled(bool val) 
	{
		distfieldStyle->useDistanceColorMap = val;
	}

	void on_contourDepthEpsilon_valueChanged(int val) 
	{
		distfieldStyle->contourDepthEpsilon = 
			static_cast<double>(val) / 1000.0;
	}

	void on_useFatContours_toggled(bool val) 
	{ 
		distfieldStyle->useFatContours = val;
	}

	void on_depthCueRange_valueChanged(int val) 
	{
		distfieldStyle->depthCueRange = 
			static_cast<double>(val) / 1000.0;
	}

	void on_useFog_toggled(bool val) 
	{
		distfieldStyle->useFog = val;
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
		if (renderer) renderer->eyeSpacing = 
			static_cast<double>(val) / 1000.0;
	}

	void on_lightOffsetDirection_valueChanged(int val)
	{
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		renderer->lightOffsetDirection = val;
		// TODO: do the same for all SimpleRenderers
	}

	void on_lightRelativeToCamera_toggled(bool val)
	{
		ui.lightOffsetDirection->setEnabled(val);
		NQVTK::Renderer *renderer = ui.nqvtkwidget->GetRenderer();
		renderer->lightRelativeToCamera = val;
		// TODO: do the same for all SimpleRenderers
	}
};
