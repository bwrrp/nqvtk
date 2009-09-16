#pragma once

#include "NQVTK/Math/Vector3.h"

#include <string>
#include <map>

class GLProgram;
class GLTextureManager;

namespace NQVTK 
{
	class ParamSet;

	class Renderable
	{
	public:
		Renderable();
		virtual ~Renderable();

		virtual void PushTransforms() const;
		virtual void PopTransforms() const;

		virtual void Draw() const = 0;

		void GetBounds(
			double &xmin, double &xmax, 
			double &ymin, double &ymax,
			double &zmin, double &zmax) const;
		void GetBounds(double bounds[6]) const;

		Vector3 GetCenter() const;

		bool visible;
		Vector3 color;
		double opacity;

		// TODO: Do transformations properly
		Vector3 position;
		double rotateX;
		double rotateY;

		ParamSet *GetParamSet(const std::string &name);
		void SetParamSet(const std::string &name, ParamSet *params);

		void ApplyParamSets(GLProgram *program, GLTextureManager *tm);
		void ApplyParamSetsArrays(GLProgram *program, 
			GLTextureManager *tm, int objectId);

	protected:
		double bounds[6];

		typedef std::map<std::string, ParamSet*> ParamSetsType;
		ParamSetsType paramSets;

	private:
		// Not implemented
		Renderable(const Renderable&);
		void operator=(const Renderable&);
	};
}
