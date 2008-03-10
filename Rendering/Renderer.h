#pragma once

#include "GLBlaat/GL.h"
#include "Camera.h"
#include "Renderable.h"
#include <vector>
#include <QTime>

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLOcclusionQuery.h"
#include "GLBlaat/GLUtility.h"

namespace NQVTK 
{
	class Renderer
	{
	public:
		Renderer() : camera(0), fbo1(0), fbo2(0), scribe(0), painter(0), query(0) { };
		virtual ~Renderer() 
		{ 
			DeleteAllRenderables();
			if (camera) delete camera;

			if (fbo1) delete fbo1;
			if (fbo2) delete fbo2;
			if (scribe) delete scribe;
			if (painter) delete painter;
			if (query) delete query;
		}

		virtual bool Initialize()
		{
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			if (camera) delete camera;
			camera = new Camera();

			if (fbo1) delete fbo1;
			fbo1 = 0;
			if (fbo2) delete fbo2;
			fbo2 = 0;

			// Set up shader programs
			if (scribe) delete scribe;
			if (painter) delete painter;
			// - Scribe (info pass)
			{
				scribe = GLProgram::New();
				bool res = scribe->AddFragmentShader(
					"uniform sampler2DRectShadow depthBuffer;"
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  float r1 = shadow2DRect(depthBuffer, r0.xyz).x;"
					"  r1 = r1 - 0.5;"
					"  if( r1 < 0.0) { discard; }"
					"  gl_FragColor = gl_Color;"
					"}");
				if (res) res = scribe->Link();
				if (!res)
				{
					qDebug("Failed to build Scribe!");
					return false;
				}
			}
			// - Painter (shading pass)
			/*
			{
				painter = GLProgram::New();
				bool res = painter->AddFragmentShader(
					"");
				if (res) res = painter->Link();
				if (!res)
				{
					qDebug("Failed to build Painter!");
					return false;
				}
			}
			*/


			if (query) delete query;
			query = GLOcclusionQuery::New();

			return true;
		}

		GLFramebuffer *CreateFBO(int w, int h)
		{
			GLFramebuffer *fbo = GLFramebuffer::New(w, h);
			fbo->CreateDepthTextureRectangle();
			fbo->CreateColorTexture();
			if (!fbo->IsOk()) qDebug("WARNING! fbo not ok!");
			fbo->Unbind();

			return fbo;
		}

		virtual void Resize(int w, int h)
		{
			glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);

			if (!fbo1) 
			{
				fbo1 = CreateFBO(w, h);
			}
			else
			{
				if (!fbo1->Resize(w, h)) qDebug("WARNING! fbo1 resize failed!");
			}

			if (!fbo2) 
			{
				fbo2 = CreateFBO(w, h);
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
			tex->BindToCurrent();
			glEnable(tex->GetTextureTarget());
			if (tex->GetTextureTarget() == GL_TEXTURE_RECTANGLE_ARB)
			{
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, tex->GetHeight());
				glVertex3d(xmin, ymin, 0.0);
				glTexCoord2d(tex->GetWidth(), tex->GetHeight());
				glVertex3d(xmax, ymin, 0.0);
				glTexCoord2d(tex->GetWidth(), 0.0);
				glVertex3d(xmax, ymax, 0.0);
				glTexCoord2d(0.0, 0.0);
				glVertex3d(xmin, ymax, 0.0);
				glEnd();
			}
			else 
			{
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 1.0);
				glVertex3d(xmin, ymin, 0.0);
				glTexCoord2d(1.0, 1.0);
				glVertex3d(xmax, ymin, 0.0);
				glTexCoord2d(1.0, 0.0);
				glVertex3d(xmax, ymax, 0.0);
				glTexCoord2d(0.0, 0.0);
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

			// Add some silly rotation while we don't have interactive camera control
			QTime midnight = QTime(0, 0);
			glRotated(static_cast<double>(QTime::currentTime().msecsTo(midnight)) / 30.0, 
				0.0, 1.0, 0.0);

			// Prepare for rendering
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Clear();
			glDisable(GL_CULL_FACE);
			glEnable(GL_COLOR_MATERIAL);
			fbo1->Bind();

			// Depth peeling
			glClearColor(0.0, 0.0, 0.0, 0.0);
			bool done = false;
			int layer = 0;
			int maxlayers = 20;
			while (!done)
			{
				GLTexture *depthBuffer = 0;
				if (layer > 0)
				{
					// Get the results of the previous layer
					depthBuffer = fbo2->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);

					depthBuffer->BindToCurrent();
					GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
						glTexParameteri(depthBuffer->GetTextureTarget(), 
						GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

					// Set up peeling shader
					scribe->Start();
					scribe->UseTexture("depthBuffer", 0);
				}
				
				Clear();

				query->Start();

				DrawRenderables();

				query->Stop();

				if (layer > 0)
				{
					scribe->Stop();

					glTexParameteri(depthBuffer->GetTextureTarget(), 
						GL_TEXTURE_COMPARE_MODE, GL_NONE);
					depthBuffer->UnbindCurrent();
				}

				SwapFramebuffers();

				// Blend results to screen
				fbo1->Unbind();
				GLTexture *colorBuffer = fbo2->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				glDisable(GL_LIGHTING);
				glDisable(GL_DEPTH_TEST);
				// TODO: we should test for this extension / GL version 1.4
				glBlendFuncSeparate(
					GL_DST_ALPHA, GL_ONE, 
					GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
				glColor3d(1.0, 1.0, 1.0);
				TestDrawTexture(colorBuffer, -1.0, 1.0, -1.0, 1.0);
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				glPopMatrix();
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				fbo1->Bind();

				unsigned int numfragments = query->GetResultui();
				++layer;
				done = (layer >= maxlayers || numfragments == 0);
				//if (done) qDebug("Layers: %d (%d fragments left)", layer, numfragments);
			}

			fbo1->Unbind();

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
			// Test: display all textures

			TestDrawTexture(fbo1->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT), 
				-1.0, 0.0, 0.0, 1.0);
			TestDrawTexture(fbo1->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT), 
				-1.0, 0.0, -1.0, 0.0);
			TestDrawTexture(fbo2->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT), 
				0.0, 1.0, 0.0, 1.0);
			TestDrawTexture(fbo2->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT), 
				0.0, 1.0, -1.0, 0.0);
			//*/
		}

		virtual void DrawRenderables()
		{
			// Iterate over all renderables and draw them
			for (std::vector<Renderable*>::const_iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				if ((*it)->visible)
				{
					(*it)->Draw();
				}
			}
		}

		void AddRenderable(Renderable *obj)
		{
			if (obj) renderables.push_back(obj);
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

	protected:
		Camera *camera;
		std::vector<Renderable*> renderables;

		GLFramebuffer *fbo1;
		GLFramebuffer *fbo2;
		GLProgram *scribe;
		GLProgram *painter;
		GLOcclusionQuery *query;

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
