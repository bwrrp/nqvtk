#pragma once

#include "LayeredRaycaster.h"

#include <string>

namespace NQVTK
{
	namespace Styles
	{
		class CustomLayeredRaycaster : public LayeredRaycaster
		{
		public:
			typedef Raycaster Superclass;

			CustomLayeredRaycaster();

			void SetScribeSource(
				std::string vertexSource, 
				std::string fragmentSource);
			void SetRaycasterSource(
				std::string vertexSource, 
				std::string fragmentSource);
			void SetRaycasterSource(
				std::string fragmentSource);
			void SetPainterSource(
				std::string vertexSource, 
				std::string fragmentSource);
			void SetPainterSource(
				std::string fragmentSource);

			virtual GLFramebuffer *CreateFBO(int w, int h);

			// Scribe stage peeling pass
			virtual GLProgram *CreateScribe();
			// Scribe stage raycasting pass
			virtual GLProgram *CreateRaycaster();
			// Painter stage
			virtual GLProgram *CreatePainter();

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);

		protected:
			std::string scribeVertexSource;
			std::string scribeFragmentSource;
			std::string raycasterVertexSource;
			std::string raycasterFragmentSource;
			std::string painterVertexSource;
			std::string painterFragmentSource;

		private:
			// Not implemented
			CustomLayeredRaycaster(const CustomLayeredRaycaster&);
			void operator=(const CustomLayeredRaycaster&);
		};
	}
}
