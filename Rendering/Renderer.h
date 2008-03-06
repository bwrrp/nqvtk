#pragma once

#include "GLBlaat/GL.h"
#include "Camera.h"
#include "Renderable.h"
#include <vector>
#include <QTime>

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLUtility.h"

namespace NQVTK 
{
	class Renderer
	{
	public:
		Renderer() : camera(0), fbo1(0), fbo2(0) { };
		virtual ~Renderer() 
		{ 
			DeleteAllRenderables();
			if (camera) delete camera;
		}

		virtual bool Initialize()
		{
			glClearColor(0.2f, 0.3f, 0.5f, 0.0f);

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			if (camera) delete camera;
			camera = new Camera();

			if (fbo1) delete fbo1;
			fbo1 = 0;
			if (fbo2) delete fbo2;
			fbo2 = 0;

			program = GLProgram::New();
			bool res = program->AddFragmentShader(
				"uniform sampler2DRectShadow depthBuffer;"
				"void main() {"
				"  vec4 r0 = gl_FragCoord;"
				"  float r1 = shadow2DRect(depthBuffer, r0.xyz).x;"
				"  r1 = r1 - 0.5;"
				"  if( r1 < 0.0) { discard; }"
				"  gl_FragColor = gl_Color;"
				"}");
			if (!res)
			{
				qDebug("Shader failed to compile!");
				qDebug(program->GetInfoLogs().c_str());
				return false;
			}
			if (!program->Link())
			{
				qDebug("Program failed to link!");
				return false;
			}

			return true;
		}

		virtual void Resize(int w, int h)
		{
			glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);

			if (!fbo1) 
			{
				fbo1 = GLFramebuffer::New(w, h);
				fbo1->CreateDepthTextureRectangle();
				fbo1->CreateColorTexture();
				if (!fbo1->IsOk()) qDebug("WARNING! fbo1 not ok!");
				fbo1->Unbind();
			}
			else
			{
				if (!fbo1->Resize(w, h)) qDebug("WARNING! fbo1 resize failed!");
			}

			if (!fbo2) 
			{
				fbo2 = GLFramebuffer::New(w, h);
				fbo2->CreateDepthTextureRectangle();
				fbo2->CreateColorTexture();
				if (!fbo2->IsOk()) qDebug("WARNING! fbo2 not ok!");
				fbo2->Unbind();
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

			glColor3d(1.0, 1.0, 1.0);
			// DrawRenderables();

			// Simple peeling test
			fbo1->Bind();

			glDisable(GL_CULL_FACE);
			Clear();
			DrawRenderables();

			// TODO: Begin depth peeling loop here...

			SwapFramebuffers();
			
			// Get the results
			GLTexture *depthBuffer = fbo2->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);

			depthBuffer->BindToCurrent();
			GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
			glTexParameteri(depthBuffer->GetTextureTarget(), 
				GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

			// Set up peeling shader
			program->Start();
			program->UseTexture("depthBuffer", 0);

			glDisable(GL_CULL_FACE);
			Clear();
			DrawRenderables();

			program->Stop();

			glTexParameteri(depthBuffer->GetTextureTarget(), 
				GL_TEXTURE_COMPARE_MODE, GL_NONE);
			depthBuffer->UnbindCurrent();

			// TODO: End depth peeling loop here...

			// TODO: Blend color buffers as we go:
			//glBlendFuncSeparateEXT(
			//	GL_DST_ALPHA, GL_ONE, 
			//	GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
			// TODO: add test for GL_EXT_blend_func_separate before we use this

			fbo1->Unbind();

			SwapFramebuffers();

			// Test: display all textures
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glDisable(GL_LIGHTING);

			TestDrawTexture(fbo1->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT), 
				-1.0, 0.0, 0.0, 1.0);
			TestDrawTexture(fbo1->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT), 
				-1.0, 0.0, -1.0, 0.0);
			TestDrawTexture(fbo2->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT), 
				0.0, 1.0, 0.0, 1.0);
			TestDrawTexture(fbo2->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT), 
				0.0, 1.0, -1.0, 0.0);
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
		GLProgram *program;

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
