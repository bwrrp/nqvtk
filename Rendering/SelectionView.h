#pragma once

#include "Scene.h"

#include <set>

namespace NQVTK
{
	// A view of a Scene that only shows selected Renderables
	// NOTE: Views do not own the base Scene because it may be shared
	class SelectionView : public Scene
	{
	public:
		typedef Scene Superclass;

		SelectionView(Scene *baseScene);

		virtual void SetVisibility(unsigned int i, bool visible);
		virtual bool GetVisibility(unsigned int i);

	protected:
		Scene *baseScene;
		std::set<unsigned int> selection;
	};
}
