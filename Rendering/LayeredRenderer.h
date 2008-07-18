#pragma once

#include "GLBlaat/GL.h"

#include "Renderer.h"

#include "Styles/RenderStyle.h"

#include <QObject> // for qDebug

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLOcclusionQuery.h"

#include <cassert>

namespace NQVTK 
{
	class LayeredRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		LayeredRenderer() : tm(0), style(0), 
			fbo1(0), fbo2(0), fboTarget(0), 
			scribe(0), painter(0), query(0) 
		{ 
			fboTarget = 0;

			maxLayers = 8;
			drawBackground = true;
		};

		virtual ~LayeredRenderer() 
		{ 
			if (tm) delete tm;
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

			if (!tm)
			{
				tm = GLTextureManager::New();
				if (!tm)
				{
					qDebug("Failed to create texture manager! Check hardware requirements...");
					return false;
				}
			}
			tm->BeginNewPass();

			if (!style) 
			{
				qDebug("No style set! Can not initialize renderer!");
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
				qDebug("Failed to build Scribe!");
				return false;
			}
			// - Painter (shading pass)
			painter = style->CreatePainter();
			if (!painter) 
			{
				qDebug("Failed to build Painter!");
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
				if (!fbo1->Resize(w, h)) qDebug("WARNING! fbo1 resize failed!");
			}

			if (!fbo2) 
			{
				fbo2 = style->CreateFBO(w, h);
			}
			else
			{
				if (!fbo2->Resize(w, h)) qDebug("WARNING! fbo2 resize failed!");
			}
			if (fboTarget)
			{
				if (!fboTarget->Resize(w, h)) qDebug("WARNING! fboTarget resize failed!");
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

		virtual void Draw()
		{
			DrawCamera();
			UpdateLighting();

			// Prepare for rendering
			if (fboTarget)
			{
				fboTarget->Bind();
			}
			else
			{
				glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
			}
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Clear();
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glDisable(GL_CULL_FACE);
			glEnable(GL_COLOR_MATERIAL);
			if (fboTarget)
			{
				fboTarget->Unbind();
			}
			fbo1->Bind();

			// TODO: we could initialize info buffers here (and swap)
			// this way shaders can skip the check for layer 0
			// TODO: handle camera being inside of objects 
			// this could be done using this method, possibly by pre-rendering 
			// layers using depth clamping / modified clipping planes...

			// Depth peeling
			bool done = false;
			int layer = 0;
			while (!done)
			{
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

				// Blend results to screen or to target FBO
				fbo1->Unbind();
				if (fboTarget)
				{
					fboTarget->Bind();
				}

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

				unsigned int numfragments = query->GetResultui();
				++layer;
				done = (layer >= maxLayers || numfragments == 0);
				//if (done) qDebug("Layers: %d (%d fragments left)", layer, numfragments);

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
			style->PrepareForObject(scribe, objectId, renderable);
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
				Vector3 center = (*it)->GetCenter();
				glTranslated((*it)->position.x, (*it)->position.y, (*it)->position.z);
				glTranslated(center.x, center.y, center.z);
				glRotated((*it)->rotateX, 1.0, 0.0, 0.0);
				glRotated((*it)->rotateY, 0.0, 1.0, 0.0);
				glTranslated(-center.x, -center.y, -center.z);
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
			if (query) 
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

		GLFramebuffer *SetTarget(GLFramebuffer *target)
		{
			GLFramebuffer *oldTarget = this->fboTarget;
			this->fboTarget = target;
			if (target)
			{
				// Make sure it's the right size
				bool ok = target->Resize(viewportWidth, viewportHeight);
				assert(ok);
			}
			return oldTarget;
		}

		int maxLayers;
		bool drawBackground;

	protected:
		GLTextureManager *tm;

		RenderStyle *style;

		GLFramebuffer *fbo1;
		GLFramebuffer *fbo2;
		GLFramebuffer *fboTarget;
		GLProgram *scribe;
		GLProgram *painter;
		GLOcclusionQuery *query;

	private:
		// Not implemented
		LayeredRenderer(const LayeredRenderer&);
		void operator=(const LayeredRenderer&);
	};
}
