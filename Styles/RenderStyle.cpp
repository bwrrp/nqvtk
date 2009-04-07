#pragma once

#include "RenderStyle.h"

#include "Renderables/Renderable.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"

#include <map>
#include <string>
#include <sstream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	RenderStyle::RenderStyle() 
		: tm(0)
	{
	}

	// ------------------------------------------------------------------------
	RenderStyle::~RenderStyle() 
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::Initialize(GLTextureManager *tm)
	{
		this->tm = tm;
	}

	// ------------------------------------------------------------------------
	void RenderStyle::PrepareForObject(GLProgram *scribe, 
		int objectId, Renderable *renderable) 
	{
		scribe->SetUniform1i("objectId", objectId);
		// Apply all ParamSets
		renderable->ApplyParamSets(scribe);
	}

	// ------------------------------------------------------------------------
	void RenderStyle::RegisterScribeTextures(GLFramebuffer *previous)
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::UnregisterScribeTextures()
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::UpdateScribeParameters(GLProgram *scribe)
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::RegisterPainterTextures(GLFramebuffer *current, 
		GLFramebuffer *previous)
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::UnregisterPainterTextures()
	{
	}
	
	// ------------------------------------------------------------------------
	void RenderStyle::UpdatePainterParameters(GLProgram *painter)
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::DrawBackground()
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);

		glBegin(GL_QUADS);
		glColor4d(0.2, 0.2, 0.25, 0.0);
		glVertex3d(-1.0, -1.0, 0.0);
		glVertex3d(1.0, -1.0, 0.0);
		glColor4d(0.6, 0.6, 0.65, 0.0);
		glVertex3d(1.0, 1.0, 0.0);
		glVertex3d(-1.0, 1.0, 0.0);
		glEnd();

		glDisable(GL_BLEND);
	}

	// ------------------------------------------------------------------------
	void RenderStyle::SetOption(const std::string &option, 
		const std::string &value)
	{
		defines[option] = value;
	}

	// ------------------------------------------------------------------------
	bool RenderStyle::HasOption(const std::string &option)
	{
		std::map<std::string, std::string>::iterator it = 
			defines.find(option);
		return (it != defines.end());
	}

	// ------------------------------------------------------------------------
	void RenderStyle::UnsetOption(const std::string &option)
	{
		defines.erase(option);
	}

	// ------------------------------------------------------------------------
	std::string RenderStyle::AddShaderDefines(std::string shader)
	{
		std::ostringstream res;
		for (std::map<std::string, std::string>::const_iterator it = 
			defines.begin(); it != defines.end(); ++it)
		{
			res << "#define " << it->first << " " << it->second << "\n";
		}
		res << shader;
		return res.str();
	}
}
