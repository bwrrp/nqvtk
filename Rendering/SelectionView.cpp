#include "SelectionView.h"

#include <set>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SelectionView::SelectionView(Scene *baseScene) : baseScene(baseScene)
	{
	}

	// ------------------------------------------------------------------------
	void SelectionView::SetVisibility(unsigned int i, bool visible)
	{
		if (visible)
		{
			// Add objectId to selection
			selection.insert(i);
		}
		else
		{
			// Remove objectId from selection
			selection.erase(i);
		}
	}

	// ------------------------------------------------------------------------
	bool SelectionView::GetVisibility(unsigned int i)
	{
		// A renderable is visible if it exists and is part of the selection
		return (GetRenderable(i) != 0) && 
			(selection.find(i) != selection.end());
	}
}
