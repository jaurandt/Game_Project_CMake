#pragma once

#include <OGL3D/Prerequisites.h>
#include <cstddef> // Express vertex buffer sizes in bytes.

class VertexArrayObject
{
public:
	VertexArrayObject(const VertexBufferData& data);
	~VertexArrayObject();

	unsigned int getID();
	void updateVertices(const void* vertices, std::size_t size); // Replace the used prefix of the existing vertex buffer.

private:
	unsigned int m_vertexBufferID = 0;
	unsigned int m_elementBufferID = 0;
	unsigned int m_vertexArrayObjectID = 0;
	std::size_t m_vertexBufferSize = 0; // Track the capacity allocated at construction.
};

