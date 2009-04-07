#pragma once

#include <map>
#include <string>

class GLTextureManager;
class GLFramebuffer;
class GLProgram;

namespace NQVTK
{
	class Renderable;

	class RenderStyle
	{
	public:
		RenderStyle();
		virtual ~RenderStyle();

		void Initialize(GLTextureManager *tm);

		virtual GLFramebuffer *CreateFBO(int w, int h) = 0;

		virtual GLProgram *CreateScribe() = 0;
		virtual GLProgram *CreatePainter() = 0;

		virtual void PrepareForObject(GLProgram *scribe, 
			int objectId, Renderable *renderable);

		virtual void RegisterScribeTextures(GLFramebuffer *previous);
		virtual void UnregisterScribeTextures();
		virtual void UpdateScribeParameters(GLProgram *scribe);

		virtual void RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous);
		virtual void UnregisterPainterTextures();
		virtual void UpdatePainterParameters(GLProgram *painter);

		virtual void DrawBackground();

		// NOTE: caller should re-create scribe and painter after changing options
		virtual void SetOption(const std::string &option, 
			const std::string &value = "1");
		virtual bool HasOption(const std::string &option);
		virtual void UnsetOption(const std::string &option);

	protected:
		GLTextureManager *tm;

		std::map<std::string, std::string> defines;

		std::string AddShaderDefines(std::string shader);

	private:
		// Not implemented
		RenderStyle(const RenderStyle&);
		void operator=(const RenderStyle&);
	};
}
