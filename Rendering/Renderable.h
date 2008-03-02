#pragma once

#include "GLBlaat/GLResource.h"

namespace NQVTK 
{
	class Renderable : public GLResource
	{
	public:
		typedef GLResource Superclass;

		Renderable() { };

		virtual ~Renderable() { }

		virtual void Draw() = 0;

		// TODO: Transformations?

		// TODO: Something about size, bounding box, ... (useful stuff)
		void GetBounds(
			double &xmin, double &xmax, 
			double &ymin, double &ymax,
			double &zmin, double &zmax) 
		{
			xmin = this->bounds[0]; xmax = this->bounds[1];
			ymin = this->bounds[2]; ymax = this->bounds[3];
			zmin = this->bounds[4]; zmax = this->bounds[5];
		}

		void GetBounds(double bounds[6])
		{
			GetBounds(
				bounds[0], bounds[1], 
				bounds[2], bounds[3], 
				bounds[4], bounds[5]);
		}

		void GetCenter(double &x, double &y, double &z)
		{
			x = (this->bounds[0] + this->bounds[1]) / 2.0;
			y = (this->bounds[2] + this->bounds[3]) / 2.0;
			z = (this->bounds[4] + this->bounds[5]) / 2.0;
		}

		void GetCenter(double center[3])
		{
			GetCenter(center[0], center[1], center[2]);
		}

	protected:
		double bounds[6];
	};
}
