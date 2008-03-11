#pragma once

#include "GLBlaat/GL.h"
#include "Camera.h"
#include "Renderable.h"
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
		Renderer() : camera(0), fbo1(0), fbo2(0), 
			tm(0), scribe(0), painter(0), query(0) { };

		virtual ~Renderer() 
		{ 
			DeleteAllRenderables();
			if (camera) delete camera;

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

			if (camera) delete camera;
			camera = new Camera();

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
			{
				scribe = GLProgram::New();
				bool res = scribe->AddVertexShader(
					"uniform float farPlane;"
					"uniform float nearPlane;"
					"varying vec3 normal;"
					"varying vec4 color;"
					"varying float depthInCamera;"
					"void main() {"
					"  normal = normalize(gl_NormalMatrix * gl_Normal);"
					"  color = gl_Color;"
					"  vec4 pos = gl_ModelViewMatrix * gl_Vertex;"
					"  float depthRange = (farPlane - nearPlane);"
					"  depthInCamera = (-pos.z / pos.w - nearPlane) / depthRange;"
					"  gl_Position = ftransform();"
					"}");
				if (res) res = scribe->AddFragmentShader(
					"#extension GL_ARB_texture_rectangle : enable\n"
					"uniform sampler2DRectShadow depthBuffer;"
					"uniform sampler2DRect infoBuffer;"
					"uniform int layer;"
					"uniform int objectId;"
					"varying vec3 normal;"
					"varying vec4 color;"
					"varying float depthInCamera;"
					// Rounds a float to the nearest integer
					"float round(float x) {"
					"  return floor(x + 0.5);"
					"}"
					// Encodes a bit set in a float, range [0..1]
					"float setBit(float byte, int bit, bool on) {"
					"  float f = 2.0;"
					"  int N = 8;"
					"  float max = pow(f, float(N)) - 1.0;"
					"  byte = round(byte * max);"
					"  float bf = pow(f, float(bit));"
					"  float b = fract(byte / bf);"
					"  float af = bf * f;"
					"  float a = floor(byte / af);"
					"  float r = bf * b + af * a;"
					"  if (on) r += f / 2.0 * bf;"
					"  return r / max;"
					"}"
					// Packs a float in two 8 bit channels
					"vec2 encodeDepth(float depth) {"
					"  depth = clamp(depth, 0.0, 1.0);"
					"  vec2 factors = vec2(1.0, 256.0);"
					"  vec2 result = fract(depth * factors);"
					"  result.x -= result.y / factors.y;"
					"  return result;"
					"}"
					// Shader main function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					// Depth peeling
					"  if (layer > 0) {"
					"    float r1 = shadow2DRect(depthBuffer, r0.xyz).x;"
					"    r1 = r1 - 0.5;"
					"    if (r1 < 0.0) { discard; }"
					"  }"
					// Get the previous info buffer
					"  vec4 prevInfo;"
					"  if (layer > 0) {"
					"    prevInfo = texture2DRect(infoBuffer, r0.xy);"
					"  } else {"
					"    prevInfo = vec4(0.0);"
					"  }"
					// Encode identity
					"  float id = float(objectId) + 1.0;"
					"  if (id < 0.0) id = 0.0;"
					"  float identity = id / 9.0;"
					// Encode in-out mask
					"  float inOutMask = prevInfo.y;"
					"  if (objectId >= 0) {"
					"    inOutMask = setBit(inOutMask, objectId, gl_FrontFacing);"
					"  }"
					// Encode depth
					"  vec2 depthVec = encodeDepth(depthInCamera);"
					// Encode normal
					"  vec3 n = (normalize(normal) + vec3(1.0)) / 2.0;"
					// Store data
					"  gl_FragData[0] = color;"
					"  gl_FragData[1] = vec4(n, 1.0);"
					"  gl_FragData[2] = vec4(identity, inOutMask, depthVec);"
					"}");
				if (res) res = scribe->Link();
				qDebug(scribe->GetInfoLogs().c_str());
				if (!res)
				{
					qDebug("Failed to build Scribe!");
					return false;
				}
			}
			// - Painter (shading pass)
			{
				painter = GLProgram::New();
				bool res = painter->AddVertexShader(
					"void main() {"
					"  gl_Position = gl_Vertex;"
					"}");
				if (res) res = painter->AddFragmentShader(
					"#extension GL_ARB_texture_rectangle : enable\n"
					"uniform sampler2DRect normals;"
					"uniform sampler2DRect colors;"
					"uniform sampler2DRect infoPrevious;"
					"uniform sampler2DRect infoCurrent;"
					"uniform int layer;"
					"uniform float farPlane;"
					"uniform float nearPlane;"
					// Rounds a float to the nearest integer
					"float round(float x) {"
					"  return floor(x + 0.5);"
					"}"
					// Unpacks a float from two 8 bit channels
					"float decodeDepth(vec2 coded) {"
					"  vec2 factors = vec2(1.0, 0.00390625);"
					"  return dot(coded, factors);"
					"}"
					// CSG formula
					"bool CSG(float mask) {"
					"  float f = 2.0;"
					"  float N = 8.0;"
					"  mask = round(mask * (pow(f, N) - 1.0)) / f;"
					"  bool inActor0 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor1 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor2 = fract(mask) > 0.25;"
					"  return inActor0 && inActor1;"
					"}"
					// CSG formula for fogging volumes
					"bool CSGFog(float mask) {"
					"  float f = 2.0;"
					"  float N = 8.0;"
					"  mask = round(mask * (pow(f, N) - 1.0)) / f;"
					"  bool inActor0 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor1 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor2 = fract(mask) > 0.25;"
					"  return inActor0 || inActor1;"
					"}"
					// Phong shading helper
					"vec3 phongShading(vec3 matColor, vec3 normal) {"
					"  vec3 l = normalize(vec3(gl_LightSource[0].position));"
					"  vec3 n = normalize(normal);"
					"  vec3 h = normalize(gl_LightSource[0].halfVector.xyz);"
					"  float NdotL = dot(n, l);"
					"  float diffuse = 0.5 + 0.5 * min(max(NdotL, 0.0), 1.0);"
					"  float specular = 0.0;"
					"  if (NdotL >= 0.0) {"
					"    specular = pow(max(dot(n, h), 0.0), 64.0);"
					"  }"
					"  return diffuse * matColor + specular * vec3(0.4);"
					"}"
					// Main shader function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  vec3 normal = texture2DRect(normals, r0.xy).rgb;"
					"  normal = (2.0 * normal) - vec3(1.0);"
					"  vec4 color = texture2DRect(colors, r0.xy);"
					"  vec4 info0 = texture2DRect(infoCurrent, r0.xy);"
					"  vec4 info1 = texture2DRect(infoPrevious, r0.xy);"
					"  if (layer == 0) info1 = vec4(0.0);"
					// Apply lighting
					"  vec3 litFragment = phongShading(color.rgb, normal);"
					// Apply CSG
					"  float mask0 = info0.y;"
					"  float mask1 = info1.y;"
					"  if (CSG(mask0) != CSG(mask1)) {"
					"    color.a = 1.0;"
					"  } else {"
					"    if (color.a > 0.0) color.a = 1.5 - length(litFragment);"
					"  }"
					// Apply contouring
					"  float contourDepthEpsilon = 0.001;"
					"  vec4 left = texture2DRect(infoCurrent, vec2(r0.x - 1.0, r0.y));"
					"  vec4 top = texture2DRect(infoCurrent, vec2(r0.x, r0.y - 1.0));"
					"  float diffH = abs(decodeDepth(left.zw) - decodeDepth(info0.zw));"
					"  float diffV = abs(decodeDepth(top.zw) - decodeDepth(info0.zw));"
					"  bool contourH = (left.x != info0.x && diffH < contourDepthEpsilon);"
					"  bool contourV = (top.x != info0.x && diffV < contourDepthEpsilon);"
					"  if (contourH || contourV) {"
					"    litFragment = vec3(0.0);"
					"    color.a = 1.0;"
					"  }"
					// Apply fogging
					"  if (CSGFog(mask1)) {"
					"    float depthCueRange = 10.0;"
					"    vec3 fogColor = vec3(1.0, 0.0, 0.2);"
					"    float depthRange = (farPlane - nearPlane);"
					"    float front = decodeDepth(info1.zw) * depthRange;"
					"    float back = decodeDepth(info0.zw) * depthRange;"
					"    float diff = back - front;"
					"    float fogAlpha = 1.0 - "
					"      clamp(exp(-diff / depthCueRange), 0.0, 1.0);"
					"    litFragment = fogColor * fogAlpha + "
					"      litFragment * color.a * (1.0 - fogAlpha);"
					"    color.a = fogAlpha + color.a * (1.0 - fogAlpha);"
					"    litFragment /= color.a;"
					"  }"
					// Pre-multiply colors by alpha
					"  litFragment *= color.a;"
					"  gl_FragColor = vec4(litFragment, color.a);"
					"}");
				if (res) res = painter->Link();
				qDebug(painter->GetInfoLogs().c_str());
				if (!res)
				{
					qDebug("Failed to build Painter!");
					return false;
				}
			}

			if (query) delete query;
			query = GLOcclusionQuery::New();

			return true;
		}

		GLFramebuffer *CreateFBO(int w, int h)
		{
			GLFramebuffer *fbo = GLFramebuffer::New(w, h);
			fbo->CreateDepthTextureRectangle();
			int nBufs = 3;
			GLenum bufs[] = {
				GL_COLOR_ATTACHMENT0_EXT, 
				GL_COLOR_ATTACHMENT1_EXT, 
				GL_COLOR_ATTACHMENT2_EXT, 
			};
			for (int i = 0; i < nBufs; ++i)
			{
				fbo->CreateColorTextureRectangle(bufs[i]);
				GLUtility::SetDefaultColorTextureParameters(
					fbo->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT));
			}
			glDrawBuffers(nBufs, bufs);
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
			glColor3d(1.0, 1.0, 1.0);
			glDisable(GL_BLEND);
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

				if (layer > 0)
				{
					// Get the previous layer's depth buffer
					depthBuffer = fbo2->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
					depthBuffer->BindToCurrent();
					GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
						glTexParameteri(depthBuffer->GetTextureTarget(), 
						GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
					scribe->UseTexture("depthBuffer", 0);
					// Get the previous layer's info buffer
					infoBuffer = fbo2->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
					glActiveTexture(GL_TEXTURE1);
					infoBuffer->BindToCurrent();
					glActiveTexture(GL_TEXTURE0);
					scribe->UseTexture("infoBuffer", 1);
				}
				
				Clear();

				query->Start();

				DrawRenderables();

				query->Stop();

				scribe->Stop();

				if (layer > 0)
				{
					// Unbind textures
					glActiveTexture(GL_TEXTURE1);
					infoBuffer->UnbindCurrent();
					glActiveTexture(GL_TEXTURE0);
					glTexParameteri(depthBuffer->GetTextureTarget(), 
						GL_TEXTURE_COMPARE_MODE, GL_NONE);
					depthBuffer->UnbindCurrent();
				}

				SwapFramebuffers();

				// Blend results to screen
				fbo1->Unbind();
				
				GLTexture *colors = fbo2->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				GLTexture *normals = fbo2->GetTexture2D(GL_COLOR_ATTACHMENT1_EXT);
				GLTexture *infoCurrent = fbo2->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				GLTexture *infoPrevious = fbo1->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);

				glDisable(GL_DEPTH_TEST);
				glBlendFuncSeparate(
					GL_DST_ALPHA, GL_ONE, 
					GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);

				glActiveTexture(GL_TEXTURE3);
				infoPrevious->BindToCurrent();
				glActiveTexture(GL_TEXTURE2);
				infoCurrent->BindToCurrent();
				glActiveTexture(GL_TEXTURE1);
				normals->BindToCurrent();
				glActiveTexture(GL_TEXTURE0);
				colors->BindToCurrent();

				painter->Start();
				painter->SetUniform1i("layer", layer);
				painter->SetUniform1f("farPlane", camera->farZ);
				painter->SetUniform1f("nearPlane", camera->nearZ);
				painter->UseTexture("colors", 0);
				painter->UseTexture("normals", 1);
				painter->UseTexture("infoCurrent", 2);
				painter->UseTexture("infoPrevious", 3);

				glColor3d(1.0, 1.0, 1.0);
				glBegin(GL_QUADS);
				glVertex3d(-1.0, -1.0, 0.0);
				glVertex3d(1.0, -1.0, 0.0);
				glVertex3d(1.0, 1.0, 0.0);
				glVertex3d(-1.0, 1.0, 0.0);
				glEnd();

				painter->Stop();

				glActiveTexture(GL_TEXTURE3);
				infoPrevious->UnbindCurrent();
				glActiveTexture(GL_TEXTURE2);
				infoCurrent->UnbindCurrent();
				glActiveTexture(GL_TEXTURE1);
				normals->UnbindCurrent();
				glActiveTexture(GL_TEXTURE0);
				colors->UnbindCurrent();

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);

				unsigned int numfragments = query->GetResultui();
				++layer;
				done = (layer >= maxlayers || numfragments == 0);
				//if (done) qDebug("Layers: %d (%d fragments left)", layer, numfragments);
				
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

	protected:
		Camera *camera;
		std::vector<Renderable*> renderables;

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
