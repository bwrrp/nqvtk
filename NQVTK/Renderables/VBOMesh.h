#pragma once

#include "Renderable.h"

#include <map>
#include <string>

namespace NQVTK
{
	class AttributeSet;

	class VBOMesh : public Renderable
	{
	public:
		typedef Renderable Superclass;

		VBOMesh();
		virtual ~VBOMesh();
		
		void BindAttributes() const;
		void UnbindAttributes() const;

		void SetupAttributes(
			const std::vector<GLAttributeInfo> requiredAttribs);

		void AddAttributeSet(const std::string &name, 
			AttributeSet *attribSet, bool isCustom = false);
		AttributeSet *GetAttributeSet(const std::string &name);
		AttributeSet *ReplaceAttributeSet(const std::string &name, 
			AttributeSet *attribSet);

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
