#pragma once

#include <OGL3D/Prerequisites.h>

class UniformBuffer
{
public:
	UniformBuffer(const UniformBufferData& data);
	~UniformBuffer();

	void setData(void* data);
	unsigned int getID();

private:
	unsigned int m_id = 0;
	unsigned int m_size = 0;
};