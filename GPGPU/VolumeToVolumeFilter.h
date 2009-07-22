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
			virtual ~VolumeToVolumeFilter();

			virtual bool Setup(Volume *input);

			virtual Volume *Execute(Volume *outVol = 0) = 0;

		protected:
			GLTextureManager *tm;
			GLProgram *program;
			Volume *input;

			virtual GLProgram *CreateProgram() = 0;
		};
	}
}
