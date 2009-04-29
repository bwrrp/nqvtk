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

			virtual Volume *Execute(Volume *outVol = 0);

			float scale;

		protected:
			GLTextureManager *tm;
			GLProgram *program;
			Volume *input;
			Volume *helperYZX;
			Volume *helperZXY;

			GLProgram *CreateProgram();
			void ExecutePass(Volume *input, Volume *output);
		};
	}
}
