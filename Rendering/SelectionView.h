#pragma once

#include "View.h"

#include <set>

namespace NQVTK
{
	// A view that only shows selected Renderables
	class SelectionView : public View
	{
	public:
		typedef View Superclass;

		SelectionView(Scene *scene);

		virtual void SetVisibility(unsigned int i, bool visible);
		virtual bool GetVisibility(unsigned int i);

	protected:
		Scene *scene;
		std::set<unsigned int> selection;
	};
}
