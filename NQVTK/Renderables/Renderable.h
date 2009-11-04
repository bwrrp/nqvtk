#pragma once

#include "NQVTK/Math/Vector3.h"

// For GLAttributeInfo
#include "GLBlaat/GLShaderInfo.h"

#include <vector>
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

		virtual void ApplyParamSets(GLProgram *program, GLTextureManager *tm);
		virtual void ApplyParamSetsArrays(GLProgram *program, 
			GLTextureManager *tm, int objectId);

		// To be implemented by subclasses that provide custom attributes
		virtual void SetupAttributes(
			const std::vector<GLAttributeInfo> requiredAttribs);

		// TODO: condense state setup into a single consistent mechanism

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
