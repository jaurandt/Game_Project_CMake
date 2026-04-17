#pragma once

#include <OGL3D/Prerequisites.h>
#include <OGL3D/Math/Vec4.h>
#include <OGL3D/Math/Rect.h>

// Needs to be set at CMake level
#ifndef UNICODE
#define UNICODE
#endif

class GraphicsEngine
{
public:
	GraphicsEngine();
	~GraphicsEngine();

	VertexArrayObjectPtr createVertexArrayObject(const VertexBufferData& data);
	UniformBufferPtr createUniformBuffer(const UniformBufferData& data);
	ShaderProgramPtr createShaderProgram(const ShaderProgramPath& path);
	
	void clear(const Vec4& color);
	void setViewport(const Rect& size);
	void setVertexArrayObject(const VertexArrayObjectPtr& vao);
	void setUniformBuffer(const UniformBufferPtr& uniformBuffer, unsigned int bindingSlot);
	void setShaderProgram(const ShaderProgramPtr& shaderProgram);
	void drawTriangles(const TriangleType& triangleType, unsigned int vertexCount, unsigned int offset);
};

