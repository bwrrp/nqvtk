#pragma once

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"

#include <map>
#include <string>
#include <sstream>

namespace NQVTK
{
	class Renderable;

	class RenderStyle
	{
	public:
		RenderStyle() 
		{ 
			tm = 0;
		}
		virtual ~RenderStyle() { }

		void Initialize(GLTextureManager *tm)
		{
			this->tm = tm;
		}

		virtual GLFramebuffer *CreateFBO(int w, int h) = 0;

		virtual GLProgram *CreateScribe() = 0;
		virtual GLProgram *CreatePainter() = 0;

		virtual void PrepareForObject(GLProgram *scribe, 
			int objectId, Renderable *renderable) 
		{
			scribe->SetUniform1i("objectId", objectId);
			// Apply all ParamSets
			for (std::map<std::string, ParamSet*>::iterator it = renderable->paramSets.begin();
				it != renderable->paramSets.end(); ++it)
			{
				it->second->SetupProgram(scribe);
			}
		}

		virtual void RegisterScribeTextures(GLFramebuffer *previous) { }
		virtual void UnregisterScribeTextures() { }
		virtual void UpdateScribeParameters(GLProgram *scribe) { }

		virtual void RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) { }
		virtual void UnregisterPainterTextures() { }
		virtual void UpdatePainterParameters(GLProgram *painter) { }

		virtual void DrawBackground()
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

		// NOTE: caller should re-create scribe and painter after changing options
		virtual void SetOption(const std::string &option, 
			const std::string &value = "1")
		{
			defines[option] = value;
		}

		virtual bool HasOption(const std::string &option)
		{
			std::map<std::string, std::string>::iterator it = 
				defines.find(option);
			return (it != defines.end());
		}

		virtual void UnsetOption(const std::string &option)
		{
			defines.erase(option);
		}

	protected:
		GLTextureManager *tm;

		std::map<std::string, std::string> defines;

		std::string AddShaderDefines(std::string shader)
		{
			std::ostringstream res;
			for (std::map<std::string, std::string>::const_iterator it = defines.begin();
				it != defines.end(); ++it)
			{
				res << "#define " << it->first << " " << it->second << "\n";
			}
			res << shader;
			return res.str();
		}

	private:
		// Not implemented
		RenderStyle(const RenderStyle&);
		void operator=(const RenderStyle&);
	};
}
