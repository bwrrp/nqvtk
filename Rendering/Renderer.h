#pragma once

#include "GLBlaat/GL.h"

#include "Camera.h"
#include "Renderable.h"
#include "Styles/RenderStyle.h"

#include <vector>

#include <QObject>

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLOcclusionQuery.h"
#include "GLBlaat/GLUtility.h"

namespace NQVTK 
{
	class Renderer
	{
	public:
		Renderer() : camera(0), style(0), 
			fbo1(0), fbo2(0), tm(0), 
			scribe(0), painter(0), query(0) { };

		virtual ~Renderer() 
		{ 
			DeleteAllRenderables();
			if (camera) delete camera;
			if (style) delete style;

			if (fbo1) delete fbo1;
			if (fbo2) delete fbo2;
			if (tm) delete tm;
			if (scribe) delete scribe;
			if (painter) delete painter;
			if (query) delete query;
		}

		virtual bool Initialize()
		{
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			if (!camera) camera = new Camera();

			if (!style) 
			{
				qDebug("No style set! Can not initialize renderer!");
				return false;
			}

			if (fbo1) delete fbo1;
			fbo1 = 0;
			if (fbo2) delete fbo2;
			fbo2 = 0;

			if (tm) delete tm;
			tm = GLTextureManager::New();
			if (!tm) 
			{
				qDebug("Failed to create texture manager!");
				return false;
			}

			// Set up shader programs
			if (scribe) delete scribe;
			if (painter) delete painter;
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
			this->width = w;
			this->height = h;

			glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);

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
		}

		virtual void Clear()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void TestDrawTexture(GLTexture *tex, 
			double xmin, double xmax, 
			double ymin, double ymax)
		{
			if (!tex) return;
			glColor3d(1.0, 1.0, 1.0);
			glDisable(GL_BLEND);
			tex->BindToCurrent();
			glEnable(tex->GetTextureTarget());
			if (tex->GetTextureTarget() == GL_TEXTURE_RECTANGLE_ARB)
			{
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 0.0);
				glVertex3d(xmin, ymin, 0.0);
				glTexCoord2d(tex->GetWidth(), 0.0);
				glVertex3d(xmax, ymin, 0.0);
				glTexCoord2d(tex->GetWidth(), tex->GetHeight());
				glVertex3d(xmax, ymax, 0.0);
				glTexCoord2d(0.0, tex->GetHeight());
				glVertex3d(xmin, ymax, 0.0);
				glEnd();
			}
			else 
			{
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 0.0);
				glVertex3d(xmin, ymin, 0.0);
				glTexCoord2d(1.0, 0.0);
				glVertex3d(xmax, ymin, 0.0);
				glTexCoord2d(1.0, 1.0);
				glVertex3d(xmax, ymax, 0.0);
				glTexCoord2d(0.0, 1.0);
				glVertex3d(xmin, ymax, 0.0);
				glEnd();
			}
			glDisable(tex->GetTextureTarget());
			tex->UnbindCurrent();
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
			// TODO: replace this, add camera reset to focus on all renderables
			Renderable *renderable = renderables[0];
			// TODO: add a useful way to focus camera on objects
			camera->FocusOn(renderable);

			// Set up the camera (matrices)
			camera->Draw();

			// Prepare for rendering
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Clear();
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glDisable(GL_CULL_FACE);
			glEnable(GL_COLOR_MATERIAL);
			fbo1->Bind();

			// TODO: we could initialize info buffers here (and swap)

			// Depth peeling
			bool done = false;
			int layer = 0;
			int maxlayers = 6;
			while (!done)
			{
				GLTexture *depthBuffer = 0;
				GLTexture *infoBuffer = 0;

				// Start Scribe program
				scribe->Start();
				scribe->SetUniform1i("layer", layer);
				scribe->SetUniform1f("farPlane", camera->farZ);
				scribe->SetUniform1f("nearPlane", camera->nearZ);

				style->BindScribeTextures(scribe, fbo2);
				
				Clear();

				query->Start();

				DrawRenderables();

				query->Stop();

				scribe->Stop();

				style->UnbindScribeTextures();

				// Blend results to screen
				fbo1->Unbind();
				
				glDisable(GL_DEPTH_TEST);
				glBlendFuncSeparate(
					GL_DST_ALPHA, GL_ONE, 
					GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);

				painter->Start();
				painter->SetUniform1i("layer", layer);
				painter->SetUniform1f("farPlane", camera->farZ);
				painter->SetUniform1f("nearPlane", camera->nearZ);

				style->BindPainterTextures(painter, fbo1, fbo2);

				glColor3d(1.0, 1.0, 1.0);
				glBegin(GL_QUADS);
				glVertex3d(-1.0, -1.0, 0.0);
				glVertex3d(1.0, -1.0, 0.0);
				glVertex3d(1.0, 1.0, 0.0);
				glVertex3d(-1.0, 1.0, 0.0);
				glEnd();

				painter->Stop();

				style->UnbindPainterTextures();

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);

				unsigned int numfragments = query->GetResultui();
				++layer;
				done = (layer >= maxlayers || numfragments == 0);
				//if (done) qDebug("Layers: %d (%d fragments left)", layer, numfragments);
				
				SwapFramebuffers();
				if (!done) fbo1->Bind();
			}

			// Blend in the background last
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glDisable(GL_LIGHTING);
			glEnable(GL_BLEND);

			glBegin(GL_QUADS);
			glColor3d(0.2, 0.2, 0.25);
			glVertex3d(-1.0, -1.0, 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glColor3d(0.6, 0.6, 0.65);
			glVertex3d(1.0, 1.0, 0.0);
			glVertex3d(-1.0, 1.0, 0.0);
			glEnd();

			glDisable(GL_BLEND);

			/*
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
		}

		virtual void DrawRenderables()
		{
			int objectId = 0;
			// Iterate over all renderables and draw them
			for (std::vector<Renderable*>::const_iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				if ((*it)->visible)
				{
					scribe->SetUniform1i("objectId", objectId);
					(*it)->Draw();
				}
				++objectId;
			}
		}

		void AddRenderable(Renderable *obj)
		{
			if (obj) renderables.push_back(obj);
		}

		Renderable *GetRenderable(unsigned int i)
		{
			if (i >= renderables.size()) return 0;
			return renderables[i];
		}

		void DeleteAllRenderables()
		{
			for (std::vector<Renderable*>::iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				delete *it;
			}
			renderables.clear();
		}

		Camera *GetCamera() { return camera; }

		bool SetStyle(RenderStyle *style) 
		{ 
			if (this->style) delete this->style;
			this->style = style;
			// Re-initialize if we're initialized
			if (camera) 
			{
				bool ok = Initialize();
				Resize(width, height);
				return ok;
			}
			return true;
		}

	protected:
		int width;
		int height;
		Camera *camera;
		std::vector<Renderable*> renderables;

		RenderStyle *style;

		GLFramebuffer *fbo1;
		GLFramebuffer *fbo2;
		GLTextureManager *tm;
		GLProgram *scribe;
		GLProgram *painter;
		GLOcclusionQuery *query;

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
