#pragma once

class GLProgram;
class GLTexture3D;
class GLTextureManager;

namespace NQVTK
{
	namespace GPGPU
	{
		class VolumeToVolumeFilter
		{
		public:
			VolumeToVolumeFilter();
			
			bool Setup(GLTexture3D *input);

			GLTexture3D *Execute();

		protected:
			GLTextureManager *tm;
			GLProgram *program;
			GLTexture3D *input;

			GLProgram *CreateProgram();
			void ExecutePass(GLTexture3D *input, GLTexture3D *output);
		};
	}
}
