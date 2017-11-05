/*
  ==============================================================================

    Shape.h
    Created: 5 Nov 2017 4:29:51pm
    Author:  srejv

  ==============================================================================
*/

#pragma once

    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
        that we can draw.
    */
struct Shape
{
    Shape (OpenGLContext& openGLContext)
    {
        if (shapeFile.load (BinaryData::teapot_obj).wasOk())
            for (int i = 0; i < shapeFile.shapes.size(); ++i)
                vertexBuffers.add (new VertexBuffer (openGLContext, *shapeFile.shapes.getUnchecked(i)));

    }

    void draw (OpenGLContext& openGLContext, Attributes& glAttributes)
    {
        for (int i = 0; i < vertexBuffers.size(); ++i)
        {
            VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
            vertexBuffer.bind();

            glAttributes.enable (openGLContext);
            glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
            glAttributes.disable (openGLContext);
        }
    }

private:
    struct VertexBuffer
    {
        VertexBuffer (OpenGLContext& context, WavefrontObjFile::Shape& aShape) : openGLContext (context)
        {
            numIndices = aShape.mesh.indices.size();

            openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
            openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

            Array<Vertex> vertices;
            createVertexListFromMesh (aShape.mesh, vertices, Colours::green);

            openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                                    static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                                                    vertices.getRawDataPointer(), GL_STATIC_DRAW);

            openGLContext.extensions.glGenBuffers (1, &indexBuffer);
            openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                                    static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
                                                    aShape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
        }

        ~VertexBuffer()
        {
            openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
            openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
        }

        void bind()
        {
            openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
            openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        }

        GLuint vertexBuffer, indexBuffer;
        int numIndices;
        OpenGLContext& openGLContext;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
    };

    WavefrontObjFile shapeFile;
    OwnedArray<VertexBuffer> vertexBuffers;

    static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
    {
        const float scale = 0.2f;
        WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
        WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

        for (int i = 0; i < mesh.vertices.size(); ++i)
        {
            const WavefrontObjFile::Vertex& v = mesh.vertices.getReference (i);

            const WavefrontObjFile::Vertex& n
                    = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;

            const WavefrontObjFile::TextureCoord& tc
                    = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;

            Vertex vert =
            {
                { scale * v.x, scale * v.y, scale * v.z, },
                { scale * n.x, scale * n.y, scale * n.z, },
                { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                { tc.x, tc.y }
            };

            list.add (vert);
        }
    }
};