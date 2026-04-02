#pragma once

#include <memory>

class VertexArrayObject;
class UniformBuffer;
class ShaderProgram;

using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
using ShaderProgramPtr = std::shared_ptr<ShaderProgram>;

struct VertexAttribute
{
	unsigned int elements = 0;
};

struct VertexBufferData
{
	void* verticesList = nullptr;
	void* indicesList = nullptr;

	unsigned int vertexSize = 0; // Size of a single vertex in bytes
	unsigned int vertexCount = 0; // Number of vertices in the list
	unsigned int indexCount = 0; // Number of indices in the list

	VertexAttribute* attributesList = nullptr;
	unsigned int attributesListSize = 0;
};

struct ShaderProgramPath
{
	const wchar_t* vertexShaderFilePath;
	const wchar_t* fragmentShaderFilePath;
};

struct UniformBufferData
{
	unsigned int size = 0;
};

enum class TriangleType 
{
	TriangleList = 0,
	TriangleStrip
};

enum class ShaderType
{
	VertexShader = 0,
	FragmentShader
};

enum class InputMouse
{
	LeftButtonDown,
	RightButtonDown,
	NoButtonDown
};