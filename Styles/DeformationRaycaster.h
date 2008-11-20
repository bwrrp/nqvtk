#pragma once

#include "Raycaster.h"

namespace NQVTK
{
	namespace Styles
	{
		class DeformationRaycaster : public NQVTK::Styles::Raycaster
		{
		public:
			typedef NQVTK::Styles::Raycaster Superclass;

			DeformationRaycaster()
			{
				testParam = 0.5;

				SetOption("NQVTK_RAYCASTER_VOLUMECOUNT", "4");

				//UnsetOption("NQVTK_RAYCASTER_CENTRALDIFFERENCES");
				UnsetOption("NQVTK_RAYCASTER_LIGHTING");

				//SetOption("NQVTK_RAYCASTER_STRIPING");
				//SetOption("NQVTK_RAYCASTER_DEFORM");
				//SetOption("NQVTK_RAYCASTER_SMEAR");
			}

			virtual GLProgram *CreatePainter()
			{
				GLProgram *painter = GLProgram::New();
				// Painter vertex shaders
				bool res = painter->AddVertexShader(
					Shaders::GenericPainterVS);
				// Painter fragment shaders
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::DeformationRaycasterPainterFS));
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

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				Superclass::UpdatePainterParameters(painter);

				painter->SetUniform1f("testParam", testParam);
			}

			float testParam;

		private:
			// Not implemented
			DeformationRaycaster(const DeformationRaycaster&);
			void operator=(const DeformationRaycaster&);
		};
	}
}
