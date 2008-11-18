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
				SetOption("NQVTK_RAYCASTER_GAUSSIANJACOBIAN");
				//SetOption("NQVTK_RAYCASTER_VECTORFIELD");
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
			}
		};
	}
}
