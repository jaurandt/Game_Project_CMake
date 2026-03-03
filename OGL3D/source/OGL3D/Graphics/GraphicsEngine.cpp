#include <OGL3D/Graphics/GraphicsEngine.h>
#include <OGL3D/Graphics/VertexArrayObject.h>
#include <OGL3D/Graphics/UniformBuffer.h>
#include <OGL3D/Graphics/ShaderProgram.h>
#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <assert.h>
#include <stdexcept>

GraphicsEngine::GraphicsEngine()
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //***CS_OWNDC recommended for OpenGL***(ADDED)***
    wcex.lpfnWndProc = DefWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    //wcex.hInstance = hInstance;
    //wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = /*(HBRUSH)(COLOR_WINDOW + 1);*/ (HBRUSH)GetStockObject(NULL_BRUSH); //***No background for OpenGL***(ADDED)***
    //wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_APPLICATION);
    wcex.lpszClassName = L"OpenGLDummyWindowClass";
    //wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    auto classID = RegisterClassExW(&wcex);
    assert(classID);

    auto dummyWindow = CreateWindowExW(0, MAKEINTATOM(classID), L"OpenGL Dummy Window", WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wcex.hInstance, nullptr);

	assert(dummyWindow);

    auto dummyDC = GetDC(dummyWindow);
    
    PIXELFORMATDESCRIPTOR pixFormDesc = {};
	pixFormDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixFormDesc.nVersion = 1;
	pixFormDesc.iPixelType = PFD_TYPE_RGBA;
	pixFormDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixFormDesc.cColorBits = 32;
	pixFormDesc.cAlphaBits = 8;
	pixFormDesc.cDepthBits = 24;
	pixFormDesc.cStencilBits = 8;
	pixFormDesc.iLayerType = PFD_MAIN_PLANE;

    auto pixelFormat = ChoosePixelFormat(dummyDC, &pixFormDesc);
	SetPixelFormat(dummyDC, pixelFormat, &pixFormDesc);

    auto dummyContext = wglCreateContext(dummyDC);
	assert(dummyContext);

	wglMakeCurrent(dummyDC, dummyContext);

    if(!gladLoadWGL(dummyDC))
    {
		throw std::runtime_error("Graphics Engine Error: gladLoadWGL failed");
	}

    if(!gladLoadGL())
    {
        throw std::runtime_error("Graphics Engine Error: gladLoadGL failed");
    }

    wglMakeCurrent(dummyDC, 0);
    wglDeleteContext(dummyContext);
    ReleaseDC(dummyWindow, dummyDC);
	DestroyWindow(dummyWindow);
}

GraphicsEngine::~GraphicsEngine()
{
}

VertexArrayObjectPtr GraphicsEngine::createVertexArrayObject(const VertexBufferData& data)
{
    return std::make_shared<VertexArrayObject>(data);
}

UniformBufferPtr GraphicsEngine::createUniformBuffer(const UniformBufferData& data)
{
	return std::make_shared<UniformBuffer>(data);
}

ShaderProgramPtr GraphicsEngine::createShaderProgram(const ShaderProgramPath& path)
{
	return std::make_shared<ShaderProgram>(path);
}

void GraphicsEngine::clear(const Vec4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsEngine::setViewport(const Rect& size)
{
	glViewport(size.left, size.top, size.width, size.height);
}

void GraphicsEngine::setVertexArrayObject(const VertexArrayObjectPtr& vao)
{
    glBindVertexArray(vao->getID());
}

void GraphicsEngine::setUniformBuffer(const UniformBufferPtr& uniformBuffer, unsigned int bindingSlot)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingSlot, uniformBuffer->getID());
}

void GraphicsEngine::setShaderProgram(const ShaderProgramPtr& shaderProgram)
{
    glUseProgram(shaderProgram->getID());
}

void GraphicsEngine::drawTriangles(const TriangleType& triangleType, unsigned int vertexCount, unsigned int offset)
{
    auto glTriType = GL_TRIANGLES;

    switch(triangleType)
    {
        case TriangleList:
            glTriType = GL_TRIANGLES;
            break;
        case TriangleStrip:
            glTriType = GL_TRIANGLE_STRIP;
            break;
        default:
            glTriType = GL_TRIANGLES;
            break;
	}
    
    glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

    glDrawElements(glTriType, vertexCount, GL_UNSIGNED_INT, 0);
    //glDrawArrays(glTriType, offset, vertexCount);
}

/*************************
Graphics Pipeline:
1) Vertex Processing
2) Primitive Assembly
3) Rasterization
4) Fragment Shader
5) Per-sample Processing
**************************/