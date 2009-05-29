#pragma once

#include <map>
#include <string>

class GLTextureManager;
class GLFramebuffer;
class GLProgram;

namespace NQVTK
{
	class Renderable;
	class View;

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

		virtual void SceneChanged(View *view);

		// TODO: caller should re-create shaders after changing options
		void SetOption(const std::string &option);
		template <typename T> void SetOption(
			const std::string &option, const T &value);
		virtual bool HasOption(const std::string &option);
		virtual void UnsetOption(const std::string &option);

		bool DoShadersNeedUpdate();
		void ShadersUpdated();

	protected:
		GLTextureManager *tm;
		bool shadersNeedUpdate;
		std::map<std::string, std::string> defines;

		std::string AddShaderDefines(std::string shader);

	private:
		// Not implemented
		RenderStyle(const RenderStyle&);
		void operator=(const RenderStyle&);
	};
}
