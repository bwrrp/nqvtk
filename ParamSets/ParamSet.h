#pragma once

#include <string>

class GLProgram;
class GLTextureManager;

namespace NQVTK
{
	class ParamSet
	{
	public:
		ParamSet();

		// TODO: implement a caching mechanism later for uniform locations

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		// TODO: implement a caching mechanism later for sampler ids

		virtual void SetupTextures(GLTextureManager *tm);
		virtual void SetupTextureArrays(GLTextureManager *tm, int objectId);

	protected:
		std::string GetArrayName(const std::string &baseName, int index);

	private:
		// Not implemented
		ParamSet(const ParamSet&);
		void operator=(const ParamSet&);
	};
}
