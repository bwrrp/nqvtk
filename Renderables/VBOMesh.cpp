#include "VBOMesh.h"

#include "AttributeSet.h"

#include "GLBlaat/GLBuffer.h"
#include "GLBlaat/GLProgram.h"

#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	VBOMesh::VBOMesh() 
	{
	}

	// ------------------------------------------------------------------------
	VBOMesh::~VBOMesh() 
	{
		// Delete the attribute sets
		for (AttributeMap::iterator it = attributeSets.begin(); 
			it != attributeSets.end(); ++it)
		{
			delete it->second;
		}
	}

	// ------------------------------------------------------------------------
	void VBOMesh::BindAttributes() const
	{
		for (AttributeMap::const_iterator it = attributeSets.begin(); 
			it != attributeSets.end(); ++it)
		{
			it->second->Bind();
		}
	}

	// ------------------------------------------------------------------------
	void VBOMesh::UnbindAttributes() const
	{
		for (AttributeMap::const_iterator it = attributeSets.begin(); 
			it != attributeSets.end(); ++it)
		{
			it->second->Unbind();
		}
	}

	// ------------------------------------------------------------------------
	void VBOMesh::SetupAttributes(
		const std::vector<GLAttributeInfo> requiredAttribs)
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
							std::cerr << "Error! Attribute " << it->name << 
								" has unsupported type " << it->type << std::endl;
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------------
	void VBOMesh::AddAttributeSet(const std::string &name, 
		AttributeSet *attribSet, bool isCustom)
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

	// ------------------------------------------------------------------------
	AttributeSet *VBOMesh::GetAttributeSet(const std::string &name)
	{
		AttributeMap::iterator it = attributeSets.find(name);
		if (it != attributeSets.end())
		{
			return it->second;
		}
		return 0;
	}
}
