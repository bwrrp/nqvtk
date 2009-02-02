#pragma once

#include "GLBlaat/GL.h"
#include "GLBlaat/GLResource.h"

#include "Math/Vector3.h"

#include "ParamSet.h"

#include <string>
#include <map>

namespace NQVTK 
{
	class Renderable : public GLResource
	{
	public:
		typedef GLResource Superclass;

		Renderable() 
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

		virtual ~Renderable() 
		{
			// Delete all ParamSets
			for (std::map<std::string, ParamSet*>::iterator it = paramSets.begin();
				it != paramSets.end(); ++it)
			{
				delete it->second;
			}
		}

		virtual void PushTransforms() const
		{
			// TODO: handle local transformations (position, orientation)
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			Vector3 center = GetCenter();
			glTranslated(position.x, position.y, position.z);
			glTranslated(center.x, center.y, center.z);
			glRotated(rotateX, 1.0, 0.0, 0.0);
			glRotated(rotateY, 0.0, 1.0, 0.0);
			glTranslated(-center.x, -center.y, -center.z);
		}

		virtual void PopTransforms() const
		{
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}

		virtual void Draw() const = 0;

		void GetBounds(
			double &xmin, double &xmax, 
			double &ymin, double &ymax,
			double &zmin, double &zmax) const
		{
			xmin = this->bounds[0]; xmax = this->bounds[1];
			ymin = this->bounds[2]; ymax = this->bounds[3];
			zmin = this->bounds[4]; zmax = this->bounds[5];
		}

		void GetBounds(double bounds[6]) const
		{
			// TODO: bounds should take transformations into account
			GetBounds(
				bounds[0], bounds[1], 
				bounds[2], bounds[3], 
				bounds[4], bounds[5]);
		}

		Vector3 GetCenter() const
		{
			// TODO: center should take transformations into account
			return Vector3(
				(this->bounds[0] + this->bounds[1]) / 2.0, 
				(this->bounds[2] + this->bounds[3]) / 2.0, 
				(this->bounds[4] + this->bounds[5]) / 2.0);
		}

		bool visible;
		Vector3 color;
		double opacity;

		// TODO: Do transformations properly
		Vector3 position;
		double rotateX;
		double rotateY;

		ParamSet *GetParamSet(const std::string &name)
		{
			ParamSetType::iterator it = paramSets.find(name);
			if (it != paramSets.end())
			{
				return it->second;
			}
			return 0;
		}

		void SetParamSet(const std::string &name, ParamSet *params)
		{
			paramSets[name] = params;
		}

		void ApplyParamSets(GLProgram *program)
		{
			for (ParamSetType::iterator it = paramSets.begin();
				it != paramSets.end(); ++it)
			{
				it->second->SetupProgram(program);
			}
		}

		void ApplyParamSetsArrays(GLProgram *program, int objectId)
		{
			for (ParamSetType::iterator it = paramSets.begin();
				it != paramSets.end(); ++it)
			{
				it->second->SetupProgramArrays(program, objectId);
			}
		}

	protected:
		double bounds[6];

		typedef std::map<std::string, ParamSet*> ParamSetType;
		ParamSetType paramSets;

	private:
		// Not implemented
		Renderable(const Renderable&);
		void operator=(const Renderable&);
	};
}
