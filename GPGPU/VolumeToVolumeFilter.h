#pragma once

class GLProgram;
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
			~VolumeToVolumeFilter();
			
			bool Setup(Volume *input);

			Volume *Execute();

			float scale;

		protected:
			GLTextureManager *tm;
			GLProgram *program;
			Volume *input;

			GLProgram *CreateProgram();
			void ExecutePass(Volume *input, Volume *output);
		};
	}
}
