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

		void SetupAttributes(const std::vector<GLAttribute> requiredAttribs)
		{
			// Clear all attributes
			for (attribMap::iterator it = this->attribs.begin(); 
				it != this->attribs.end(); ++it)
			{
				it->second->index = -1;
			}
			// Set attributes used by the program
			for (std::vector<GLAttribute>::const_iterator it = requiredAttribs.begin();
				it != requiredAttribs.end(); ++it)
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
						//qDebug("Warning! Required attribute '%s' not available!", 
						//	it->name.c_str());

						// Set a default value for the attribute
						// It seems we can ignore the size, as array elements are listed 
						// separately in requiredAttribs.
						// Matrices seem to be listed once; rows need to be set separately
						// TODO: type for mat4 is returned as vec4, how to distinguish?
						std::vector<float> zeroes;
						zeroes.resize(4 * 4);
						switch (it->type)
						{
							case GL_FLOAT:
								glVertexAttrib1fv(it->index, &zeroes[0]);
								break;

							case GL_FLOAT_MAT4x2:
								glVertexAttrib2fv(it->index + 3, &zeroes[0]);
							case GL_FLOAT_MAT3x2:
								glVertexAttrib2fv(it->index + 2, &zeroes[0]);
							case GL_FLOAT_MAT2:
								glVertexAttrib2fv(it->index + 1, &zeroes[0]);
							case GL_FLOAT_VEC2:
								glVertexAttrib2fv(it->index, &zeroes[0]);
								break;

							case GL_FLOAT_MAT4x3:
								glVertexAttrib3fv(it->index + 3, &zeroes[0]);
							case GL_FLOAT_MAT3:
								glVertexAttrib3fv(it->index + 2, &zeroes[0]);
							case GL_FLOAT_MAT2x3:
								glVertexAttrib3fv(it->index + 1, &zeroes[0]);
							case GL_FLOAT_VEC3:
								glVertexAttrib3fv(it->index, &zeroes[0]);
								break;

							case GL_FLOAT_MAT4:
								glVertexAttrib4fv(it->index + 3, &zeroes[0]);
							case GL_FLOAT_MAT3x4:
								glVertexAttrib4fv(it->index + 2, &zeroes[0]);
							case GL_FLOAT_MAT2x4:
								glVertexAttrib4fv(it->index + 1, &zeroes[0]);
							case GL_FLOAT_VEC4:
								glVertexAttrib4fv(it->index, &zeroes[0]);
								break;

							default:
								qDebug("Error! Attribute %s has unsupported type %d.", 
									it->name.c_str(), it->type);
						}
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
