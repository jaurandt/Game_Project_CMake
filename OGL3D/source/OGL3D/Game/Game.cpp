#include <OGL3D/Game/Game.h>
#include <OGL3D/Math/Vec4.h>
#include <OGL3D/Window/GWindow.h>
#include <OGL3D/Graphics/GraphicsEngine.h>
#include <OGL3D/Graphics/ShaderProgram.h>
#include <OGL3D/Graphics/UniformBuffer.h>
#include <OGL3D/Math/Mat4.h>
#include <windows.h>
#include <iostream>

struct UniformData
{
	Mat4 world;
	Mat4 view;
	Mat4 projection;
};

Game::Game() : m_graphicsEngine(std::make_unique<GraphicsEngine>()), /*should this come before m_display?*/
			   m_display(std::make_unique<GWindow>()),
			   m_currentTime(),
			   m_previousTime(),
			   m_elapsedSeconds(0.0),
			   m_projectionMatrix(nullptr),
			   m_viewMatrix(nullptr),
			   m_worldMatrix(nullptr),
			   m_trans(nullptr),
			   m_polygonVAO(nullptr),
			   m_uniformBuffer(nullptr),
			   m_shaderProgram(nullptr)
{
	m_display->makeCurrentContext();

	m_graphicsEngine->setViewport(m_display->getInnerSize());
}

Game::~Game()
{
}

void Game::onCreate()
{
	//m_graphicsEngine->clear(Vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//m_display->present(false);
	/*
	const float triangleVertices[] = {
		 -0.5f,  -0.5f,  0.0f,
		  1.0f,   0.0f,  0.0f,

		  0.5f,  -0.5f,  0.0f,
		  0.0f,   1.0f,  0.0f,

		  0.0f,   0.5f,  0.0f,
		  0.0f,   0.0f,  1.0f
	};
	*/
	/*
	const float polygonVertices[] = {
		 -0.5f,  -0.5f,  0.0f,
		  1.0f,   0.0f,  0.0f,
		  
		 -0.5f,   0.5f,  0.0f,
		  0.0f,   1.0f,  0.0f,
		  
		  0.5f,  -0.5f,  0.0f,
		  0.0f,   0.0f,  1.0f,
		 
		  0.5f,   0.5f,  0.0f,
		  1.0f,   1.0f,  0.0f
	};
	*/
	
	//Cube vertices with position and color attributes
	const float vertices[] = {
		-0.5f, -0.5f,  0.5f, // 0
		 0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, // 1
		 0.0f,  0.0f,  1.0f,

		 0.5f,  0.5f,  0.5f, // 2
		 0.0f,  0.0f,  1.0f,

		 0.5f, -0.5f,  0.5f, // 3
		 0.0f,  0.0f,  1.0f,

		 0.5f, -0.5f, -0.5f, // 4
		 0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f, -0.5f, // 5
		 0.0f,  1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f, // 6
		 0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  // 7
		 0.0f,  1.0f,  0.0f

	};
	
	const unsigned int indices[] = {
		0, 3, 2, //front face
		2, 1, 0, 
		
		3, 4, 5, //right face
		5, 2, 3,
		
		4, 7, 6, //back face
		6, 5, 4,
		
		7, 0, 1, //left face 
		1, 6, 7,
		
		1, 2, 5, //top face
		5, 6, 1,
		
		7, 4, 3, //bottom face
		3, 0, 7
	};
	
	VertexAttribute polyAttributes[] = {
		3, // Position
		3  // Color
	};

	m_polygonVAO = m_graphicsEngine->createVertexArrayObject({
		(void*)vertices,
		(void*)indices,
		sizeof(float) * (3 + 3), //3 position elements, 3 color, vertexSize
		8, // number of vertices, vertexCount
		36, // number of indices, indexCount
		polyAttributes,
		2 // number of attributes
	});

	m_uniformBuffer = m_graphicsEngine->createUniformBuffer({
		sizeof(UniformData)
	});

	m_shaderProgram = m_graphicsEngine->createShaderProgram({L"Assets/Shaders/BasicShader.vert", L"Assets/Shaders/BasicShader.frag"});
	//m_graphicsEngine->setShaderProgram(m_shaderProgram);
	m_shaderProgram->setUniformBufferSlot("UniformData", 0);

	auto windowSize = m_display->getInnerSize();
	float aspect = 1.0f;
	if(windowSize.height != 0)
	{
		aspect = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
	}

	float fovRadians = 45.0f * (3.1415927f / 180.0f);
	
	/////////////////////////////////////////////////////////////////////////
	//*****TO DO: Create an Object class that has its own matrices and data//
	/////////////////////////////////////////////////////////////////////////
	m_projectionMatrix = std::make_unique<Mat4>();
	m_viewMatrix = std::make_unique<Mat4>();
	m_worldMatrix = std::make_unique<Mat4>();
	m_trans = std::make_unique<Mat4>();
	
	m_projectionMatrix->setPerspective(fovRadians, aspect, 0.01f, 100.0f);
	
	m_viewMatrix->setLookAt(
		Vec4(0.0f, 0.0f, 3.0f, 1.0f), // Eye
		Vec4(0.0f, 0.0f, 0.0f, 1.0f), // Center
		Vec4(0.0f, 1.0f, 0.0f, 1.0f)  // Up
	);
}

void Game::onUpdate(InputMouse mouse)
{
	m_currentTime = std::chrono::high_resolution_clock::now();

	if (m_previousTime.time_since_epoch().count())
	{
		m_elapsedSeconds = m_currentTime - m_previousTime;
	}
	
	m_previousTime = m_currentTime;

	auto deltaTime = static_cast<float>(m_elapsedSeconds.count());

	//rotate camera around the cube's center point while left mouse is held
	if (mouse == InputMouse::LeftButtonDown)
	{
		m_scale = m_scale + (3.1415927f * deltaTime);

		m_sinScale = sinf(m_scale);
		m_cosScale = cosf(m_scale);

		m_viewMatrix->setLookAt(
			Vec4(3.0f * m_sinScale, 0.0f, 3.0f * m_cosScale, 1.0f), // Eye
			Vec4(0.0f, 0.0f, 0.0f, 1.0f), // Center
			Vec4(0.0f, 1.0f, 0.0f, 1.0f)  // Up
		);
	}
	
	//Mat4 worldMatrix, trans;
	
	/////////////////////////////////
	//SCALE MATRIX///////////////////
	/////////////////////////////////
	//trans.setIdentity();
	//trans.setScale(Vec4(1.0f, 1.0f, 1.0f, 1.0f)); //sineScale
	//worldMatrix *= trans;

	/////////////////////////////////
	//TRANSLATION MATRIX/////////////
	/////////////////////////////////
	//trans.setIdentity();
	//trans.setTranslation(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	//worldMatrix *= trans;
	
	/////////////////////////////////
	//ROTATION MATRIX AROUND X AXIS//
	/////////////////////////////////
	//m_trans->setIdentity();
	//m_trans->setRotationX(0.053);
	//*m_worldMatrix *= *m_trans;

	/////////////////////////////////
	//ROTATION MATRIX AROUND Y AXIS//
	/////////////////////////////////
	//m_trans->setIdentity();
	//m_trans->setRotationY(0.053);
	//*m_worldMatrix *= *m_trans;
	
	UniformData data = {*m_worldMatrix, *m_viewMatrix, *m_projectionMatrix};
	m_uniformBuffer->setData(&data);

	m_graphicsEngine->clear(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	
	m_graphicsEngine->setVertexArrayObject(m_polygonVAO);
	m_graphicsEngine->setUniformBuffer(m_uniformBuffer, 0);
	m_graphicsEngine->setShaderProgram(m_shaderProgram);
	m_graphicsEngine->drawTriangles(TriangleType::TriangleList, 36, 0);
	
	m_display->present(true);
}

void Game::onQuit()
{
}

void Game::Run()
{
	onCreate();
	MSG msg;
	InputMouse mouse = InputMouse::NoButtonDown;

	while(m_isRunning)
	{
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
				case WM_QUIT:
				{
					m_isRunning = false;
					continue;
				}
				case WM_LBUTTONDOWN:
				{
					mouse = InputMouse::LeftButtonDown;
					break;
				}
				case WM_LBUTTONUP:
				{
					mouse = InputMouse::NoButtonDown;
					break;
				}
				default: 
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		
		onUpdate(mouse);
	}

	onQuit();
}

void Game::Quit()
{
	m_isRunning = false;
}