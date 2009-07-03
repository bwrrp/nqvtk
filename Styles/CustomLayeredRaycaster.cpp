#include "CustomLayeredRaycaster.h"

#include "GLBlaat/GLFrameBuffer.h"
#include "GLBlaat/GLProgram.h"

#include "Shaders.h"

#include <iostream>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		CustomLayeredRaycaster::CustomLayeredRaycaster()
		{
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetScribeSource(
				std::string vertexSource, 
				std::string fragmentSource)
		{
			this->scribeVertexSource = vertexSource;
			this->scribeFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetRaycasterSource(
				std::string vertexSource, 
				std::string fragmentSource)
		{
			this->raycasterVertexSource = vertexSource;
			this->raycasterFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetPainterSource(
				std::string vertexSource, 
				std::string fragmentSource)
		{
			this->painterVertexSource = vertexSource;
			this->painterFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		GLFramebuffer *CustomLayeredRaycaster::CreateFBO(int w, int h)
		{
			// TODO: create default 4x RGBA float infobuffer
			return 0;
		}

		// --------------------------------------------------------------------
		GLProgram *CustomLayeredRaycaster::CreateScribe()
		{
			GLProgram *scribe = GLProgram::New();
			// Scribe vertex shaders
			bool res = scribe->AddVertexShader(
				AddShaderDefines(scribeVertexSource));
			// Scribe fragment shaders
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(scribeFragmentSource));
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
			if (res) res = scribe->Link();
			std::cout << scribe->GetInfoLogs() << std::endl;
			if (!res)
			{
				delete scribe;
				return 0;
			}
			return scribe;
		}

		// --------------------------------------------------------------------
		GLProgram *CustomLayeredRaycaster::CreateRaycaster()
		{
			return 0;
		}

		// --------------------------------------------------------------------
		GLProgram *CustomLayeredRaycaster::CreatePainter()
		{
			return 0;
		}

		// --------------------------------------------------------------------

		// --------------------------------------------------------------------
	}
}
