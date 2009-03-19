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
		class LayeredIsoRaycaster : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			LayeredIsoRaycaster()
			{
			}
		};
	}
}
