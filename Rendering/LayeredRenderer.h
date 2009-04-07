#pragma once

#include "GLBlaat/GL.h"

#include "Renderer.h"

#include "Renderables/VBOMesh.h"

#include "Styles/RenderStyle.h"

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLOcclusionQuery.h"

#include <cassert>
#include <iostream>

namespace NQVTK 
{
	class LayeredRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		LayeredRenderer() : style(0), 
			fbo1(0), fbo2(0), 
			scribe(0), painter(0), query(0) 
		{ 
			maxLayers = 8;
			skipLayers = 0;
			drawBackground = true;
		}

		virtual ~LayeredRenderer() 
		{ 
			if (style) delete style;

			if (fbo1) delete fbo1;
			if (fbo2) delete fbo2;

			if (scribe) delete scribe;
			if (painter) delete painter;
			if (query) delete query;
		}

		virtual bool Initialize()
		{
			if (!Superclass::Initialize()) return false;

			if (!style) 
			{
				std::cerr << "No style set! Can not initialize renderer!" << std::endl;
				return false;
			}
			style->Initialize(tm);

			if (fbo1) delete fbo1;
			fbo1 = 0;
			if (fbo2) delete fbo2;
			fbo2 = 0;

			// Set up shader programs
			if (scribe) delete scribe;
			if (painter) delete painter;
			scribe = painter = 0;
			// - Scribe (info pass)
			scribe = style->CreateScribe();
			if (!scribe)
			{
				std::cerr << "Failed to build Scribe!" << std::endl;
				return false;
			}
			scribeAttribs = scribe->GetActiveAttributes();
#ifndef NDEBUG
			for (std::vector<GLAttributeInfo>::const_iterator it = scribeAttribs.begin();
				it != scribeAttribs.end(); ++it)
			{
				std::cout << "Scribe uses attribute '" <<
					it->name << "' (" << it->index << ")" << std::endl;
			}
#endif
			// - Painter (shading pass)
			painter = style->CreatePainter();
			if (!painter) 
			{
				std::cerr << "Failed to build Painter!" << std::endl;
				return false;
			}

			if (query) delete query;
			query = GLOcclusionQuery::New();

			return true;
		}

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);

			if (!fbo1) 
			{
				fbo1 = style->CreateFBO(w, h);
			}
			else
			{
				if (!fbo1->Resize(w, h))
				{
					std::cerr << "WARNING! fbo1 resize failed!" << std::endl;
				}
			}

			if (!fbo2) 
			{
				fbo2 = style->CreateFBO(w, h);
			}
			else
			{
				if (!fbo2->Resize(w, h))
				{
					std::cerr << "WARNING! fbo2 resize failed!" << std::endl;
				}
			}
		}

		void SwapFramebuffers()
		{
			bool bound = fbo1->IsBound();
			if (bound)
			{
				// Swap binding
				fbo1->Unbind();
				fbo2->Bind();
			}
			// Swap fbos
			GLFramebuffer *fbo = fbo1;
			fbo1 = fbo2;
			fbo2 = fbo;
		}

		virtual void DrawScribePass(int layer)
		{
			// For scribes that don't have a vertex shader
			glEnable(GL_COLOR_MATERIAL);

			// Depth clamping "avoids" clipping problems
			// TODO: it's better to fill holes by first drawing the near clipping plane
			if (GLEW_NV_depth_clamp)
			{
				glEnable(GL_DEPTH_CLAMP_NV);
			}

			// Start Scribe program
			scribe->Start();
			scribe->SetUniform1i("layer", layer);
			scribe->SetUniform1f("farPlane", camera->farZ);
			scribe->SetUniform1f("nearPlane", camera->nearZ);
			style->UpdateScribeParameters(scribe);

			// Set up textures used by the scribe
			style->RegisterScribeTextures(fbo2);
			tm->SetupProgram(scribe);
			tm->Bind();
			
			Clear();

			query->Start();
			DrawRenderables();
			query->Stop();

			scribe->Stop();

			// Clean up references in the texture manager
			tm->Unbind();
			style->UnregisterScribeTextures();

			if (GLEW_NV_depth_clamp)
			{
				glDisable(GL_DEPTH_CLAMP_NV);
			}
		}

		virtual void DrawPainterPass(int layer)
		{
			glDisable(GL_DEPTH_TEST);
			// TODO: might want to make blend function customizable (in styles?)
			glBlendFuncSeparate(
				GL_DST_ALPHA, GL_ONE, 
				GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);

			// Start painter program
			painter->Start();
			painter->SetUniform1i("layer", layer);
			painter->SetUniform1f("farPlane", camera->farZ);
			painter->SetUniform1f("nearPlane", camera->nearZ);
			painter->SetUniform1f("viewportX", static_cast<float>(viewportX));
			painter->SetUniform1f("viewportY", static_cast<float>(viewportY));
			painter->SetUniform3f("cameraPos", camera->position.x, camera->position.y, camera->position.z);
			ApplyParamSetsArrays(painter);
			style->UpdatePainterParameters(painter);

			// Set up textures used by the painter
			style->RegisterPainterTextures(fbo1, fbo2);
			tm->SetupProgram(painter);
			tm->Bind();

			// Draw a full screen quad for the painter pass
			glColor3d(1.0, 1.0, 1.0);
			glBegin(GL_QUADS);
			glVertex3d(-1.0, -1.0, 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glVertex3d(1.0, 1.0, 0.0);
			glVertex3d(-1.0, 1.0, 0.0);
			glEnd();

			painter->Stop();

			// Clean up references in the texture manager
			tm->Unbind();
			style->UnregisterPainterTextures();

			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		}

		virtual void Draw()
		{
			DrawCamera();
			UpdateLighting();

			// Prepare target for rendering
			if (fboTarget)
			{
				fboTarget->Bind();
			}
			else
			{
				glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
			}

			// For front-to-back blending, we clear the target with alpha = 1.0
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Clear();

			if (fboTarget)
			{
				fboTarget->Unbind();
			}
			fbo1->Bind();

			// All infobuffers should be cleared to black
			glClearColor(0.0, 0.0, 0.0, 0.0);

			// Render both sides of the surface polys
			glDisable(GL_CULL_FACE);

			// TODO: we could initialize the info buffers here (and swap)
			// this way shaders can skip the check for layer 0
			// TODO: handle camera being inside of objects 
			// this could be done using this method, possibly by pre-rendering 
			// layers using depth clamping / modified clipping planes...

			// Depth peeling
			bool done = false;
			int layer = 0;
			while (!done)
			{
				// Draw the scribe pass (propagate information buffers)
				DrawScribePass(layer);

				// Blend results to screen or to target FBO
				fbo1->Unbind();
				if (fboTarget)
				{
					fboTarget->Bind();
				}

				if (layer >= skipLayers) 
				{
					// Draw the painter pass (visualize layer, blend with previous results)
					DrawPainterPass(layer);
				}

				unsigned int numfragments = query->GetResultui();
				++layer;
				done = (layer >= maxLayers || numfragments == 0);
				/* Printing layer counts slows down rendering, so disabled by default
				if (done) 
				{
					std::cout << "Layers: " << layer << 
						" (" << numfragments << " fragments left)" << std::endl;
				}
				//*/

				SwapFramebuffers();
				if (!done) 
				{
					if (fboTarget)
					{
						fboTarget->Unbind();
					}
					fbo1->Bind();
				}
			}

			// Blend in the background last
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			if (drawBackground) style->DrawBackground();

			/* Debug views
			GLenum top = GL_COLOR_ATTACHMENT0_EXT;
			GLenum bottom = GL_COLOR_ATTACHMENT2_EXT;
			//GLenum bottom = GL_DEPTH_ATTACHMENT_EXT;
			// Test: display all textures
			glDisable(GL_DEPTH_TEST);
			TestDrawTexture(fbo1->GetTexture2D(top), 
				-1.0, 0.0, 0.0, 1.0);
			TestDrawTexture(fbo1->GetTexture2D(bottom), 
				-1.0, 0.0, -1.0, 0.0);
			TestDrawTexture(fbo2->GetTexture2D(top), 
				0.0, 1.0, 0.0, 1.0);
			TestDrawTexture(fbo2->GetTexture2D(bottom), 
				0.0, 1.0, -1.0, 0.0);
			glEnable(GL_DEPTH_TEST);
			//*/

			if (fboTarget)
			{
				fboTarget->Unbind();
			}
		}

		virtual void PrepareForRenderable(int objectId, NQVTK::Renderable *renderable)
		{
			NQVTK::VBOMesh *mesh = dynamic_cast<NQVTK::VBOMesh*>(renderable);
			if (mesh) mesh->SetupAttributes(scribeAttribs);
			style->PrepareForObject(scribe, objectId, renderable);
		}

		virtual void ApplyParamSetsArrays(GLProgram *program)
		{
			int objectId = 0;
			for (std::vector<Renderable*>::const_iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				Renderable *renderable = *it;
				if (renderable)
				{
					renderable->ApplyParamSetsArrays(program, objectId);
				}
				++objectId;
			}
		}

		virtual void DrawRenderables()
		{
			int objectId = 0;
			// Load object transforms
			// HACK: should be handled elsewhere, but we also need the distfield transform
			for (std::vector<Renderable*>::const_iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				glActiveTexture(GL_TEXTURE0 + objectId);
				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				Renderable *renderable = *it;
				if (renderable)
				{
					Vector3 center = renderable->GetCenter();
					glTranslated(renderable->position.x, 
						renderable->position.y, renderable->position.z);
					glTranslated(center.x, center.y, center.z);
					glRotated(renderable->rotateX, 1.0, 0.0, 0.0);
					glRotated(renderable->rotateY, 0.0, 1.0, 0.0);
					glTranslated(-center.x, -center.y, -center.z);
				}
				++objectId;
			}
			glActiveTexture(GL_TEXTURE0);

			// Draw all objects
			Superclass::DrawRenderables();
		}

		bool SetStyle(RenderStyle *style) 
		{ 
			this->style = style;
			// Re-initialize if we're initialized
			if (query && style) 
			{
				bool ok = Initialize();
				// TODO: subclasses might need this call
				// problems occur if a subclass modifies the size, so we call this
				//LayeredRenderer::Resize(viewportWidth, viewportHeight);
				Resize(viewportWidth, viewportHeight);
				return ok;
			}
			return true;
		}

		int maxLayers;
		int skipLayers;
		bool drawBackground;

	protected:
		RenderStyle *style;
		std::vector<GLAttributeInfo> scribeAttribs;

		GLFramebuffer *fbo1;
		GLFramebuffer *fbo2;
		GLProgram *scribe;
		GLProgram *painter;
		GLOcclusionQuery *query;

	private:
		// Not implemented
		LayeredRenderer(const LayeredRenderer&);
		void operator=(const LayeredRenderer&);
	};
}
