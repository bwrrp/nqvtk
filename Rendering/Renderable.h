#pragma once

#include "GLBlaat/GLResource.h"
#include "Math/Vector3.h"

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
		}

		virtual ~Renderable() { }

		virtual void Draw() const = 0;

		// TODO: Transformations?

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
			GetBounds(
				bounds[0], bounds[1], 
				bounds[2], bounds[3], 
				bounds[4], bounds[5]);
		}

		Vector3 GetCenter() const
		{
			return Vector3(
				(this->bounds[0] + this->bounds[1]) / 2.0, 
				(this->bounds[2] + this->bounds[3]) / 2.0, 
				(this->bounds[4] + this->bounds[5]) / 2.0);
		}

		bool visible;

	protected:
		double bounds[6];

	private:
		// Not implemented
		Renderable(const Renderable&);
		void operator=(const Renderable&);
	};
}
