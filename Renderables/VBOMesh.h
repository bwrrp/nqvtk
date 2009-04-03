#pragma once

#include "Renderable.h"

#include "AttributeSet.h"

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

		VBOMesh() { }

		virtual ~VBOMesh() 
		{
			// Delete the attribute sets
			for (AttributeMap::iterator it = attributeSets.begin(); 
				it != attributeSets.end(); ++it)
			{
				delete it->second;
			}
		}

		void BindAttributes() const
		{
			for (AttributeMap::const_iterator it = attributeSets.begin(); 
				it != attributeSets.end(); ++it)
			{
				it->second->Bind();
			}
		}

		void UnbindAttributes() const
		{
			for (AttributeMap::const_iterator it = attributeSets.begin(); 
				it != attributeSets.end(); ++it)
			{
				it->second->Unbind();
			}
		}

		void SetupAttributes(const std::vector<GLAttributeInfo> requiredAttribs)
		{
			// Clear all attributes
			for (AttributeMap::iterator it = this->customAttribs.begin(); 
				it != this->customAttribs.end(); ++it)
			{
				it->second->DontUse();
			}
			// Set attributes used by the program
			for (std::vector<GLAttributeInfo>::const_iterator it = 
				requiredAttribs.begin(); it != requiredAttribs.end(); ++it)
			{
				if (it->index >= 0)
				{
					AttributeMap::iterator ait = customAttribs.find(it->name);
					if (ait != customAttribs.end())
					{
						ait->second->UseAsVertexAttrib(it->index);
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

		void AddAttributeSet(const std::string &name, AttributeSet *attribSet, bool isCustom = false)
		{
			AttributeSet *oldSet = attributeSets[name];
			assert(!oldSet);
			if (oldSet) delete oldSet;
			attributeSets[name] = attribSet;

			if (isCustom)
			{
				customAttribs[name] = attribSet;
			}
		}

		AttributeSet *GetAttributeSet(const std::string &name)
		{
			AttributeMap::iterator it = attributeSets.find(name);
			if (it != attributeSets.end())
			{
				return it->second;
			}
			return 0;
		}

		virtual void Draw() const = 0;

	protected:
		typedef std::map<std::string, AttributeSet*> AttributeMap;
		AttributeMap attributeSets;
		AttributeMap customAttribs;

	private:
		// Not implemented
		VBOMesh(const VBOMesh&);
		void operator=(const VBOMesh&);
	};
}
