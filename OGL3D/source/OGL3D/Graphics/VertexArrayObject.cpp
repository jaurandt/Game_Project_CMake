#include <OGL3D/Graphics/VertexArrayObject.h>
#include <glad/glad.h>
#include <stdexcept> // Report updates larger than the allocated buffer.

VertexArrayObject::VertexArrayObject(const VertexBufferData& data)
	: m_vertexBufferSize(static_cast<std::size_t>(data.vertexCount) * data.vertexSize) // Allocate space for the full vertex capacity.
{
	glGenBuffers(1, &m_vertexBufferID);
	glGenBuffers(1, &m_elementBufferID);

	glGenVertexArrays(1, &m_vertexArrayObjectID);
	glBindVertexArray(m_vertexArrayObjectID);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, m_vertexBufferSize, data.verticesList,
		data.dynamicVertices ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW); // Match the usage hint to the caller's update pattern.
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * data.indexCount, data.indicesList, GL_STATIC_DRAW);

	unsigned int offset = 0;
	for(unsigned int i = 0; i < data.attributesListSize; i++)
	{
		glVertexAttribPointer(
			i,
			data.attributesList[i].elements,
			GL_FLOAT,
			GL_FALSE,
			data.vertexSize,
			(void*)(uintptr_t)offset
		);
		glEnableVertexAttribArray(i);
		offset += data.attributesList[i].elements * sizeof(float);
	}
	
	glBindVertexArray(0);

	//m_vertexBufferData = data;
}

VertexArrayObject::~VertexArrayObject()
{
	glDeleteBuffers(1, &m_vertexBufferID);
	glDeleteVertexArrays(1, &m_vertexArrayObjectID);

	glDeleteBuffers(1, &m_elementBufferID);
}

unsigned int VertexArrayObject::getID()
{
	return m_vertexArrayObjectID;
}

void VertexArrayObject::updateVertices(const void* vertices, std::size_t size)
{ // Keep vertex formats and element data attached to the existing vertex array.
	if (size > m_vertexBufferSize)
	{ // Guard against writes past the original allocation.
		throw std::out_of_range("Vertex update exceeds buffer capacity"); // Report the invalid update before calling OpenGL.
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID); // Select this mesh's vertex buffer for the update.
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices); // Upload only the vertices used by this frame.
}
