#include <OGL3D/Graphics/ShaderProgram.h>
#include <glad/glad.h>
#include <fstream>
#include <sstream>

ShaderProgram::ShaderProgram(const ShaderProgramPath& path)
{
	m_programID = glCreateProgram();
	attach(path.vertexShaderFilePath, ShaderType::VertexShader);
	attach(path.fragmentShaderFilePath, ShaderType::FragmentShader);
	link();
}

ShaderProgram::~ShaderProgram()
{
	for (int i = 0; i < 2; i++)
	{
		glDetachShader(m_programID, m_attachedShaders[i]);
		glDeleteShader(m_attachedShaders[i]);
	}
	glDeleteProgram(m_programID);
}

void ShaderProgram::setUniformBufferSlot(const char* uniformName, unsigned int bindingSlot)
{
	unsigned int index = glGetUniformBlockIndex(m_programID, uniformName);
	glUniformBlockBinding(m_programID, index, bindingSlot);
}

void ShaderProgram::attach(const wchar_t* filePath, ShaderType shaderType)
{
	std::ifstream fileStream(filePath);
	std::string shaderCode;

	if (fileStream.is_open())
	{
		std::stringstream buffer;
		buffer << fileStream.rdbuf();
		shaderCode = buffer.str();
		fileStream.close();
	}
	else
	{
		throw std::runtime_error("Shader File Error: Unable to open file");
	}

	unsigned int shaderID = 0;
	if (shaderType == ShaderType::VertexShader)
	{
		shaderID = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (shaderType == ShaderType::FragmentShader)
	{
		shaderID = glCreateShader(GL_FRAGMENT_SHADER);
	}
	else
	{
		throw std::runtime_error("Shader Type Error: Unknown shader type");
	}

	auto shaderCodeCStr = shaderCode.c_str();
	glShaderSource(shaderID, 1, &shaderCodeCStr, NULL);

	glCompileShader(shaderID);

	// Check for compilation errors
	int success;
	char infoLog[512];
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
		glDeleteShader(shaderID);
		throw std::runtime_error(std::string("Shader Compilation Error: ") + infoLog);
	}

	glAttachShader(m_programID, shaderID);
	m_attachedShaders[static_cast<int>(shaderType)] = shaderID;
}

void ShaderProgram::link()
{
	glLinkProgram(m_programID);

	// Check for linking errors
	int success;
	char infoLog[512];
	glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
		throw std::runtime_error(std::string("Shader Linking Error: ") + infoLog);
	}
}
