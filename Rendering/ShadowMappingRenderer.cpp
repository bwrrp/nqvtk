#pragma once

#include "ShadowMappingRenderer.h"

#include "Camera.h"
#include "Styles/ShadowMap.h"
#include "Styles/DistanceFields.h"

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTextureManager.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	void Matrix4x4Inverse(const float (&mat)[16], float (&inv)[16]) 
	{
		// TODO: properly invert matrix, this only works for affine matrices
		// Transpose 3x3 top left 
		for (int i = 0; i < 3; ++i) 
		{
			for (int j = 0; j < 3; ++j) 
			{
				inv[i + j * 4] = mat[j + i * 4];
			}
		}
		// Rotate negated translation vector
		inv[12] = -mat[12]*inv[ 0] - mat[13]*inv[ 4] - mat[14]*inv[ 8];
		inv[13] = -mat[12]*inv[ 1] - mat[13]*inv[ 5] - mat[14]*inv[ 9];
		inv[14] = -mat[12]*inv[ 2] - mat[13]*inv[ 6] - mat[14]*inv[10];
		// Copy remaining elements
		inv[ 3] = mat[ 3];
		inv[ 7] = mat[ 7];
		inv[11] = mat[11];
		inv[15] = mat[15];
	}

	// ------------------------------------------------------------------------
	ShadowMappingRenderer::ShadowMappingRenderer() 
		: LayeredRenderer() 
	{
		shadowBuffer = 0;
		shadowStyle = 0;
		shadowRenderer = new NQVTK::LayeredRenderer();
	}

	// ------------------------------------------------------------------------
	ShadowMappingRenderer::~ShadowMappingRenderer() 
	{
		if (shadowBuffer) delete shadowBuffer;
		if (shadowStyle) delete shadowStyle;
		delete shadowRenderer;
	}

	// ------------------------------------------------------------------------
	bool ShadowMappingRenderer::Initialize()
	{
		if (!style) return false;
		style->SetOption("NQVTK_USE_SHADOWMAP");
		style->SetOption("NQVTK_USE_VSM");

		if (!Superclass::Initialize()) return false;

		// Set up the shadow renderer
		delete shadowStyle;
		shadowStyle = new NQVTK::Styles::ShadowMap(style);
		shadowRenderer->SetStyle(shadowStyle);
		return shadowRenderer->Initialize();
	}

	// ------------------------------------------------------------------------
	void ShadowMappingRenderer::SetViewport(int x, int y, int w, int h)
	{
		// Resize buffers managed by the parent class
		Superclass::SetViewport(x, y, w, h);

		// Resize the shadow renderer
		// NOTE: shadow map size is currently fixed
		shadowRenderer->Resize(1024, 1024);

		// Resize or recreate shadow buffer
		if (!shadowBuffer)
		{
			shadowBuffer = shadowStyle->CreateShadowBufferFBO(1024, 1024);
			shadowRenderer->SetTarget(shadowBuffer);
		}
		else
		{
			//if (!shadowBuffer->Resize(w, h))
			//{
			//	std::cerr << "WARNING! shadowBuffer resize failed!" << std::endl;
			//}
		}
	}

	// ------------------------------------------------------------------------
	void ShadowMappingRenderer::Draw()
	{
		// Synchronize renderer state
		shadowRenderer->SetScene(scene);
		shadowRenderer->maxLayers = maxLayers;

		// TODO: add a parallel projection camera so zoom doesn't affect shadowmap bounds
		UpdateLighting();

		shadowRenderer->GetCamera()->focus = camera->focus;
		shadowRenderer->GetCamera()->position = lightPos;

		// Draw the shadow map
		shadowRenderer->Draw();
		
		// Get the shadow map
		GLTexture *shadowMap = shadowBuffer->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
		tm->AddTexture("shadowMap", shadowMap, false);

		// Get the modelview and projection matrices for the light's camera
		shadowRenderer->DrawCamera();
		float shadowNear = static_cast<float>(
			shadowRenderer->GetCamera()->nearZ);
		float shadowFar = static_cast<float>(
			shadowRenderer->GetCamera()->farZ);
		float lmvm[16];
		float lpm[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, lmvm);
		glGetFloatv(GL_PROJECTION_MATRIX, lpm);
		// Draw the camera to get the object modelview transform
		DrawCamera();
		// Get the object modelview matrix and its inverse
		float mvm[16];
		float inv[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mvm);
		glGetFloatv(GL_PROJECTION_MATRIX, inv);
		Matrix4x4Inverse(mvm, inv);
		// Load the fourth texture matrix with the transform for shadow mapping
		// (the first two are currently used for object transforms)
		// this assumes it's not used by a renderable...
		// TODO: use custom uniforms instead
		glActiveTexture(GL_TEXTURE4);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		// - Screen coordinates to texture coordinates
		glTranslated(0.5, 0.5, 0.5);
		glScaled(0.5, 0.5, 0.5);
		// - Add the light's projection and modelview matrices
		glMultMatrixf(lpm);
		glMultMatrixf(lmvm);
		// - Finally, add the inverse modelview transform
		glMultMatrixf(inv);
		glMatrixMode(GL_MODELVIEW);

		// Set some extra shadow parameters
		scribe->Start();
		scribe->SetUniform1f("shadowNearPlane", shadowNear);
		scribe->SetUniform1f("shadowFarPlane", shadowFar);
		scribe->SetUniformMatrix4fv("shadowMVM", 1, lmvm);
		scribe->SetUniformMatrix4fv("shadowPM", 1, lpm);
		scribe->Stop();

		// Draw the normal pass
		Superclass::Draw();

		/*
		// DEBUG: show shadow buffer
		if (fboTarget) fboTarget->Bind();
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
		TestDrawTexture(shadowMap, 0.0, 1.0, 0.0, 1.0);
		if (fboTarget) fboTarget->Unbind();
		//*/
	}
}
