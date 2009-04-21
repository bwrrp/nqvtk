#pragma once

class GLProgram;
class GLTexture3D;
class GLTextureManager;

namespace NQVTK
{
	class Volume;

	namespace GPGPU
	{
		class VolumeToVolumeFilter
		{
		public:
			VolumeToVolumeFilter();
			
			bool Setup(Volume *input);

			Volume *Execute();

			float scale;

		protected:
			GLTextureManager *tm;
			GLProgram *program;
			Volume *input;

			GLProgram *CreateProgram();
			void ExecutePass(GLTexture3D *input, GLTexture3D *output);
		};
	}
}
