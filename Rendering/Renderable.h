#pragma once

namespace NQVTK 
{
	class Renderable
	{
	public:
		Renderable() { };
		virtual ~Renderable() { }

		virtual void Draw() = 0;

		// TODO: Transformations?
		// TODO: Something about size, bounding box, ... (useful stuff)
	};
}
