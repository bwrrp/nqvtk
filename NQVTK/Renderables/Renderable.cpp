#include "Renderable.h"

#include "Math/Vector3.h"
#include "ParamSets/ParamSet.h"

#include "GLBlaat/GL.h"

#include <string>
#include <map>

namespace NQVTK 
{
	// ------------------------------------------------------------------------
	Renderable::Renderable() 
	{
		for (int i = 0; i < 6; ++i)
		{
			bounds[i] = 0.0;
		}
		visible = true;

		color = Vector3(1.0, 1.0, 1.0);
		opacity = 1.0;

		rotateX = rotateY = 0.0;
	}

	// ------------------------------------------------------------------------
	Renderable::~Renderable() 
	{
		// Delete all ParamSets
		for (ParamSetsType::iterator it = paramSets.begin();
			it != paramSets.end(); ++it)
		{
			delete it->second;
		}
	}

	// ------------------------------------------------------------------------
	void Renderable::PushTransforms() const
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		Vector3 center = GetCenter();
		glTranslated(position.x, position.y, position.z);
		glTranslated(center.x, center.y, center.z);
		glRotated(rotateX, 1.0, 0.0, 0.0);
		glRotated(rotateY, 0.0, 1.0, 0.0);
		glTranslated(-center.x, -center.y, -center.z);
	}

	// ------------------------------------------------------------------------
	void Renderable::PopTransforms() const
	{
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	// ------------------------------------------------------------------------
	void Renderable::GetBounds(
		double &xmin, double &xmax, 
		double &ymin, double &ymax,
		double &zmin, double &zmax) const
	{
		xmin = this->bounds[0]; xmax = this->bounds[1];
		ymin = this->bounds[2]; ymax = this->bounds[3];
		zmin = this->bounds[4]; zmax = this->bounds[5];
	}

	// ------------------------------------------------------------------------
	void Renderable::GetBounds(double bounds[6]) const
	{
		// TODO: bounds should take transformations into account
		GetBounds(
			bounds[0], bounds[1], 
			bounds[2], bounds[3], 
			bounds[4], bounds[5]);
	}

	// ------------------------------------------------------------------------
	Vector3 Renderable::GetCenter() const
	{
		// TODO: center should take transformations into account
		return Vector3(
			(this->bounds[0] + this->bounds[1]) / 2.0, 
			(this->bounds[2] + this->bounds[3]) / 2.0, 
			(this->bounds[4] + this->bounds[5]) / 2.0);
	}

	// ------------------------------------------------------------------------
	ParamSet *Renderable::GetParamSet(const std::string &name)
	{
		ParamSetsType::iterator it = paramSets.find(name);
		if (it != paramSets.end())
		{
			return it->second;
		}
		return 0;
	}

	// ------------------------------------------------------------------------
	void Renderable::SetParamSet(const std::string &name, ParamSet *params)
	{
		// Delete any existing paramset with the same name
		ParamSetsType::iterator it = paramSets.find(name);
		if (it != paramSets.end())
		{
			if (it->second != params) delete it->second;
			if (!params) paramSets.erase(it);
		}
		if (params) paramSets[name] = params;
	}

	// ------------------------------------------------------------------------
	void Renderable::ApplyParamSets(GLProgram *program, GLTextureManager *tm)
	{
		for (ParamSetsType::iterator it = paramSets.begin();
			it != paramSets.end(); ++it)
		{
			if (program) it->second->SetupProgram(program);
			if (tm) it->second->SetupTextures(tm);
		}
	}

	// ------------------------------------------------------------------------
	void Renderable::ApplyParamSetsArrays(GLProgram *program, 
		GLTextureManager *tm, int objectId)
	{
		for (ParamSetsType::iterator it = paramSets.begin();
			it != paramSets.end(); ++it)
		{
			if (program) it->second->SetupProgramArrays(program, objectId);
			if (tm) it->second->SetupTextureArrays(tm, objectId);
		}
	}

	// ------------------------------------------------------------------------
	void Renderable::SetupAttributes(
		const std::vector<GLAttributeInfo> requiredAttribs)
	{
	}
}
