#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLUtility.h"

namespace NQVTK
{
	namespace Styles
	{
		// TODO: we may want to create an abstract superclass for raycaster styles
		// (they have an extra stage)
		class LayeredRaycaster : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			LayeredRaycaster()
			{
			}

			// Scribe stage peeling pass
			virtual GLProgram *CreateScribe()
			{
				GLProgram *scribe = GLProgram::New();
				// Scribe vertex shaders
				bool res = scribe->AddVertexShader(
					AddShaderDefines(Shaders::LayeredRaycasterScribePeelVS));
				// Scribe fragment shaders
				if (res) res = scribe->AddFragmentShader(
					AddShaderDefines(Shaders::LayeredRaycasterScribePeelFS));
				if (res) res = scribe->AddFragmentShader(
					AddShaderDefines(Shaders::LibUtility));
				if (res) res = scribe->Link();
				qDebug(scribe->GetInfoLogs().c_str());
				if (!res)
				{
					delete scribe;
					return 0;
				}
				return scribe;
			}

			// Scribe stage raycasting pass
			virtual GLProgram *CreateRaycaster()
			{
				GLProgram *raycaster = GLProgram::New();
				// Raycaster vertex shaders
				bool res = raycaster->AddVertexShader(
					Shaders::GenericPainterVS);
				// Raycaster fragment shaders
				if (res) res = raycaster->AddFragmentShader(
					AddShaderDefines(Shaders::LayeredRaycasterScribeCastFS));
				if (res) res = raycaster->Link();
				qDebug(raycaster->GetInfoLogs().c_str());
				if (!res) 
				{
					delete raycaster;
					return 0;
				}
				return raycaster;
			}

			// Painter stage
			virtual GLProgram *CreatePainter()
			{
				GLProgram *painter = GLProgram::New();
				// Painter vertex shaders
				bool res = painter->AddVertexShader(
					Shaders::GenericPainterVS);
				// Painter fragment shaders
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::LayeredRaycasterPainterFS));
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::LibUtility));
				if (res) res = painter->Link();
				qDebug(painter->GetInfoLogs().c_str());
				if (!res) 
				{
					delete painter;
					return 0;
				}
				return painter;
			}

		private:
			// Not implemented
			LayeredRaycaster(const LayeredRaycaster&);
			void operator=(const LayeredRaycaster&);
		};
	}
}
