#include "PCAPointCorrespondenceGlyphs.h"
#include "VBOMesh.h"
#include "AttributeSet.h"

#include "ParamSets/PCAParamSet.h"
#include "ParamSets/PCACorrespondenceParamSet.h"

#include "GLBlaat/GLBuffer.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <cassert>

#define BUFFER_OFFSET(i) ((char *)0 + (i))

namespace NQVTK
{
	// ------------------------------------------------------------------------
	void GetSetData(AttributeSet *set, float* data)
	{
		// TODO: add support for other data types
		assert(set->GetValueType() == GL_FLOAT);

		int numTuples = set->GetNumberOfTuples();
		int numComps = set->GetNumberOfComponents();

		float *mapped = reinterpret_cast<float*>(
			set->GetBuffer()->Map(GL_READ_ONLY));
		memcpy(data, mapped, numTuples * numComps * sizeof(float));
		set->GetBuffer()->Unmap();
		set->GetBuffer()->Unbind();
	}

	// ------------------------------------------------------------------------
	AttributeSet *MergeSets(VBOMesh *obj1, VBOMesh *obj2, 
		const std::string &name)
	{
		// Extract points from the objects
		AttributeSet *set1 = obj1->GetAttributeSet(name);
		AttributeSet *set2 = obj2->GetAttributeSet(name);

		// Make sure the objects have the same number of values
		int numValues1 = set1->GetNumberOfTuples() * 
			set1->GetNumberOfComponents();
		int numValues2 = set2->GetNumberOfTuples() * 
			set2->GetNumberOfComponents();
		assert(numValues1 == numValues2);

		// We can only map a single VBO at a time, so first copy to main memory

		// Copy points from each object
		float *data = new float[numValues1 + numValues2];
		GetSetData(set1, data);
		GetSetData(set2, data + numValues1);

		// Create a new VBO
		AttributeSet *combined = new AttributeSet(
			GL_FLOAT, set1->GetNumberOfComponents());
		combined->SetData(
			set1->GetNumberOfTuples() + set2->GetNumberOfTuples(), data);

		delete [] data;

		return combined;
	}

	// ------------------------------------------------------------------------
	PCAPointCorrespondenceGlyphs::PCAPointCorrespondenceGlyphs(
		VBOMesh *obj1, VBOMesh *obj2)
	{
		// Merge points
		AttributeSet *points = MergeSets(obj1, obj2, "gl_Vertex");
		points->UseAsVertices();
		AddAttributeSet("gl_Vertex", points);

		// Now copy the eigenvectors
		int nummodes1 = PCAParamSet::GetNumEigenModes(obj1);
		int nummodes2 = PCAParamSet::GetNumEigenModes(obj2);
		assert(nummodes1 == nummodes2);

		for (int i = 0; i < nummodes1; ++i)
		{
			std::ostringstream name;
			name << "eigvecs[" << i << "]";
			AttributeSet *vecset = MergeSets(obj1, obj2, name.str());
			AddAttributeSet(name.str(), vecset, true);
		}

		// Lines are created between corresponding points on the two objects
		numLines = obj1->GetAttributeSet("gl_Vertex")->GetNumberOfTuples();
		// Create objectId buffer
		float *ids = new float[numLines * 2];
		// Create index buffer for glyph lines
		lineIndices = GLBuffer::New();
		lineIndices->BindAsIndexData();
		lineIndices->SetData(numLines * 2 * sizeof(GLuint), 0, GL_STATIC_DRAW);
		unsigned int *indices = reinterpret_cast<unsigned int*>(
			lineIndices->Map(GL_WRITE_ONLY));
		// Create indices
		for (int i = 0; i < numLines; ++i)
		{
			// Line start and end points
			indices[i * 2] = i;
			indices[i * 2 + 1] = i + numLines;
			// Source ids, for determining which weights to use in the shader
			ids[i] = 0.0;
			ids[i + numLines] = 10.0;
		}
		lineIndices->Unmap();
		lineIndices->Unbind();

		AttributeSet *idset = new AttributeSet(GL_FLOAT, 1);
		idset->SetData(numLines * 2, ids);
		AddAttributeSet("sourceId", idset, true);
		delete [] ids;

		// The bounds are roughly the combined bounds for the two meshes
		// TODO: bounds change when the mesh is deformed!
		double bounds1[6];
		double bounds2[6];
		obj1->GetBounds(bounds1);
		obj2->GetBounds(bounds2);
		for (unsigned int i = 0; i < 3; ++i)
		{
			bounds[2 * i] = std::min(bounds1[2 * i], bounds2[2 * i]);
			bounds[2 * i + 1] = std::max(bounds1[2 * i + 1], bounds2[2 * i + 1]);
		}

		// Add a combined param set for the weights
		SetParamSet("pcacorrespondence", new PCACorrespondenceParamSet(
			dynamic_cast<PCAParamSet*>(obj1->GetParamSet("pca")), 
			dynamic_cast<PCAParamSet*>(obj2->GetParamSet("pca"))));
	}

	// ------------------------------------------------------------------------
	void PCAPointCorrespondenceGlyphs::Draw() const
	{
		// Color
		glColor4d(color.x, color.y, color.z, opacity);

		// Enter object coordinates
		PushTransforms();

		// Setup vbo and pointers
		BindAttributes();

		glDisable(GL_LIGHTING);
		glNormal3d(0.0, 0.0, 0.0);
		glLineWidth(2.0);

		lineIndices->BindAsIndexData();
		glDrawElements(GL_LINES, numLines * 2, 
			GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		lineIndices->Unbind();

		// Unset vbo render state
		UnbindAttributes();

		// Restore world coordinates
		PopTransforms();
	}
}
