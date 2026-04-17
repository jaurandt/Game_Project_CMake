#include <OGL3D/Graphics/UniformBuffer.h>
#include <glad/glad.h>

UniformBuffer::UniformBuffer(const UniformBufferData& data)
{
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_UNIFORM_BUFFER, m_id);
	glBufferData(GL_UNIFORM_BUFFER, data.size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	m_size = data.size;
}

UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers(1, &m_id);
}

void UniformBuffer::setData(void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_id);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, m_size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned int UniformBuffer::getID()
{
	return m_id;
}
