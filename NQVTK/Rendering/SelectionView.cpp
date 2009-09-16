#include "SelectionView.h"

#include "Renderables/Renderable.h"

#include <set>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SelectionView::SelectionView(Scene *scene) : View(scene)
	{
	}

	// ------------------------------------------------------------------------
	SelectionView::SelectionView(View *sameSceneAs) : View(sameSceneAs)
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
		// Visible means it exists, is globally visible and is in the selection
		return (selection.find(i) != selection.end()) 
			&& Superclass::GetVisibility(i);
	}
}
