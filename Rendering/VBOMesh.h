#pragma once

#include "Renderable.h"

#include "AttributePointers.h"

#include "GLBlaat/GLBuffer.h"
#include "GLBlaat/GLProgram.h"

#include <QObject> // for qDebug

#include <vector>
#include <map>
#include <string>
#include <cassert>

namespace NQVTK
{
	class VBOMesh : public Renderable
	{
	public:
		typedef Renderable Superclass;

		VBOMesh()
		{
			vertexBuffer = GLBuffer::New();
			assert(vertexBuffer);
		}

		virtual ~VBOMesh() 
		{ 
			ClearPointers();
			delete vertexBuffer;
		}

		// Pointers
		void VertexPointer(int size, GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::VertexPointer(
				size, type, stride, offset));			
		}
		void NormalPointer(GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::NormalPointer(
				type, stride, offset));
		}
		void ColorPointer(int size, GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::ColorPointer(
				size, type, stride, offset));
		}
		void TexCoordPointer(unsigned int index, int size, 
			GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::TexCoordPointer(
				index, size, type, stride, offset));
		}
		void VertexAttribPointer(std::string name, int index, 
			int size, GLenum type, bool normalized, int stride, int offset)
		{
			AttributePointers::VertexAttribPointer *vap = 
				new AttributePointers::VertexAttribPointer(
					index, size, type, normalized, stride, offset);
			pointers.push_back(vap);
			attribs[name] = vap;
		}

		void SetPointers() const
		{
			vertexBuffer->BindAsVertexData();
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				(*it)->Set();
			}
		}

		void UnsetPointers() const
		{
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				(*it)->Unset();
			}
			vertexBuffer->Unbind();
		}

		void ClearPointers()
		{
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				delete (*it);
			}
			pointers.clear();
		}

		void SetupAttributes(const std::vector<GLAttribute> attributes)
		{
			// Clear all attributes
			for (attribMap::iterator it = this->attribs.begin(); 
				it != this->attribs.end(); ++it)
			{
				it->second->index = -1;
			}
			// Set attributes used by the program
			for (std::vector<GLAttribute>::const_iterator it = attributes.begin();
				it != attributes.end(); ++it)
			{
				if (it->index >= 0)
				{
					attribMap::iterator ait = attribs.find(it->name);
					if (ait != attribs.end())
					{
						ait->second->index = it->index;
					}
					else
					{
						qDebug("Warning! Required attribute '%s' not available!", 
							it->name.c_str());
					}
				}
			}
		}

		virtual void Draw() const = 0;

	protected:
		GLBuffer *vertexBuffer;
		std::vector<AttributePointers::AttributePointer*> pointers;
		typedef std::map<std::string, AttributePointers::VertexAttribPointer*> attribMap;
		attribMap attribs;

	private:
		// Not implemented
		VBOMesh(const VBOMesh&);
		void operator=(const VBOMesh&);
	};
}
