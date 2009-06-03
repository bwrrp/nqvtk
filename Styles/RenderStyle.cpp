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
#include <iostream>

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
		std::cout << "Initializing style with " << 
			defines.size() << " options set..." << std::endl;
		this->tm = tm;
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
	void RenderStyle::SceneChanged(View *view)
	{
	}

	// ------------------------------------------------------------------------
	void RenderStyle::SetOption(const std::string &option)
	{
		SetOption(option, "1");
	}

	// ------------------------------------------------------------------------
	template <typename T>
	void RenderStyle::SetOption(const std::string &option, const T &value)
	{
		std::ostringstream str;
		str << value;
		SetOption(option, str.str());
	}

	// ------------------------------------------------------------------------
	template void RenderStyle::SetOption<int>(
		const std::string&, const int&);
	template void RenderStyle::SetOption<unsigned int>(
		const std::string&, const unsigned int&);
	template void RenderStyle::SetOption<float>(
		const std::string&, const float&);

	// ------------------------------------------------------------------------
	template <> void RenderStyle::SetOption<std::string>(
		const std::string &option, const std::string &value)
	{
		// If the value changes, we need to rebuild the shaders
		std::map<std::string, std::string>::iterator it = 
			defines.find(option);
		if (it != defines.end())
		{
			if (value != it->second)
			{
				shadersNeedUpdate = true;
			}
		}
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
	bool RenderStyle::DoShadersNeedUpdate()
	{
		return shadersNeedUpdate;
	}

	// ------------------------------------------------------------------------
	void RenderStyle::ShadersUpdated()
	{
		// TODO: reset shadersNeedUpdate automatically
		shadersNeedUpdate = false;
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
