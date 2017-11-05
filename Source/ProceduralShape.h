/*
  ==============================================================================

    ProceduralShape.h
    Created: 5 Nov 2017 4:42:04pm
    Author:  srejv

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

struct ProceduralShape {
	std::vector<float> verts = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};

	std::vector<uint32> index = {
		0,1,2
	};

	ProceduralShape(OpenGLContext& openGLContext)
	{
		vertexBuffers.add(new VertexBuffer(openGLContext, verts, index));
	}

	void draw(OpenGLContext& openGLContext, Attributes& glAttributes)
	{
		for (int i = 0; i < vertexBuffers.size(); ++i)
		{
			VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked(i);
			vertexBuffer.bind();

			glAttributes.enable(openGLContext);
			glDrawElements(GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
			glAttributes.disable(openGLContext);
		}
	}

private:
	struct VertexBuffer
	{
		VertexBuffer(OpenGLContext& context, std::vector<float>& vertData, std::vector<uint32>& indicies) : openGLContext(context)
		{
			numIndices = indicies.size();

			openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
			openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

			Array<Vertex> vertices;
			//createVertexListFromMesh (aShape.mesh, vertices, Colours::green);

			Colour colour(Colours::green);
			
			for (int i = 0; i < vertData.size(); i += 3) {
				vertices.add({
					{ vertData[i], vertData[i + 1], vertData[i + 2] },
					{ 0.0f, 1.0f, 0.0f },
					{ colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
					{ vertData[i], vertData[i + 1] }
				});
			}

			openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER,
				static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof(Vertex)),
				&vertices, GL_STATIC_DRAW);
			
			openGLContext.extensions.glGenBuffers(1, &indexBuffer);
			openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof(juce::uint32)),
				&indicies, GL_STATIC_DRAW);
		}

		~VertexBuffer()
		{
			openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
			openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
		}

		void bind()
		{
			openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		}

		GLuint vertexBuffer, indexBuffer;
		int numIndices;
		OpenGLContext& openGLContext;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VertexBuffer)
	};

	WavefrontObjFile shapeFile;
	OwnedArray<VertexBuffer> vertexBuffers;

	
};