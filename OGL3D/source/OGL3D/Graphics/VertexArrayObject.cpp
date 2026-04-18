#include <OGL3D/Graphics/VertexArrayObject.h>
#include <glad/glad.h>

VertexArrayObject::VertexArrayObject(const VertexBufferData& data)
{
	glGenBuffers(1, &m_vertexBufferID);
	glGenBuffers(1, &m_elementBufferID);

	glGenVertexArrays(1, &m_vertexArrayObjectID);
	glBindVertexArray(m_vertexArrayObjectID);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, data.vertexCount * data.vertexSize, data.verticesList, GL_STATIC_DRAW);
	
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
