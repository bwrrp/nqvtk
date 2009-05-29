#pragma once

#include "Scene.h"

#include "Renderables/Renderable.h"
#include "Math/Vector3.h"

#include <vector>
#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	Scene::Scene()
	{
	}

	// ------------------------------------------------------------------------
	Scene::~Scene()
	{
		DeleteAllRenderables();
	}

	// ------------------------------------------------------------------------
	int Scene::AddRenderable(Renderable *obj)
	{
		renderables.push_back(obj);
		return renderables.size() - 1;
	}

	// ------------------------------------------------------------------------
	Renderable *Scene::GetRenderable(unsigned int i)
	{
		if (i >= renderables.size()) return 0;
		return renderables[i];
	}

	// ------------------------------------------------------------------------
	Renderable *Scene::SetRenderable(unsigned int i, Renderable *obj)
	{
		assert(i < static_cast<unsigned int>(GetNumberOfRenderables()));
		Renderable *old = GetRenderable(i);
		renderables[i] = obj;
		return old;
	}

	// ------------------------------------------------------------------------
	unsigned int Scene::GetNumberOfRenderables() 
	{
		return renderables.size();
	}

	// ------------------------------------------------------------------------
	void Scene::DeleteAllRenderables()
	{
		for (std::vector<Renderable*>::iterator it = renderables.begin();
			it != renderables.end(); ++it)
		{
			delete *it;
		}
		renderables.clear();
	}

	// ------------------------------------------------------------------------
	void Scene::ResetRenderables()
	{
		for (std::vector<Renderable*>::iterator it = renderables.begin();
			it != renderables.end(); ++it)
		{
			(*it)->position = Vector3();
			(*it)->rotateX = 0.0;
			(*it)->rotateY = 0.0;
		}
	}

	// ------------------------------------------------------------------------
	bool Scene::GetVisibility(unsigned int i)
	{
		Renderable *renderable = GetRenderable(i);
		if (!renderable) return false;
		return renderable->visible;
	}

	// ------------------------------------------------------------------------
	void Scene::SetVisibility(unsigned int i, bool visible)
	{
		Renderable *renderable = GetRenderable(i);
		if (!renderable) return;
		renderable->visible = visible;
	}
}
