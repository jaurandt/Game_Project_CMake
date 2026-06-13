#include <OGL3D/Game/Game.h>
#include <OGL3D/Math/Vec4.h>
#include <OGL3D/Window/GWindow.h>
#include <OGL3D/Graphics/GraphicsEngine.h>
#include <OGL3D/Graphics/ShaderProgram.h>
#include <OGL3D/Graphics/UniformBuffer.h>
#include <OGL3D/Math/Mat4.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <random>

struct UniformData
{
	Mat4 world;
	Mat4 view;
	Mat4 projection;
};

namespace
{
	constexpr unsigned int SampleCount = 56;
	constexpr float Pi = 3.1415927f;
	constexpr float BoxWidth = 6.0f;
	constexpr float XMin = -BoxWidth * 0.5f;
	constexpr float XMax = BoxWidth * 0.5f;
	constexpr float ZMin = -BoxWidth * 0.5f;
	constexpr float ZMax = BoxWidth * 0.5f;
	constexpr float BaseY = -1.35f;
	constexpr float DensityScale = 0.95f;
	constexpr int ModeCount = 7;
	constexpr int ModeNx[ModeCount] = { 2, 1, 2, 3, 1, 3, 4 };
	constexpr int ModeNz[ModeCount] = { 1, 2, 2, 1, 3, 3, 2 };

	float evaluateProbability(float u, float v, int mode, float excitedMix, float phase)
	{
		const int nx = ModeNx[mode];
		const int nz = ModeNz[mode];
		const float c2 = std::clamp(excitedMix, 0.0f, 0.95f);
		const float c1 = sqrtf(1.0f - c2 * c2);
		const float groundState = sinf(Pi * u) * sinf(Pi * v);
		const float excitedState = sinf(static_cast<float>(nx) * Pi * u) *
			sinf(static_cast<float>(nz) * Pi * v);

		return c1 * c1 * groundState * groundState +
			c2 * c2 * excitedState * excitedState +
			2.0f * c1 * c2 * groundState * excitedState * cosf(phase);
	}

	void addVertex(std::vector<float>& vertices, float x, float y, float z, float r, float g, float b)
	{
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(r);
		vertices.push_back(g);
		vertices.push_back(b);
	}

	void addFloorRect(
		std::vector<float>& vertices,
		std::vector<unsigned int>& indices,
		float left,
		float right,
		float front,
		float back,
		float y,
		float r,
		float g,
		float b)
	{
		const unsigned int start = static_cast<unsigned int>(vertices.size() / 6);

		addVertex(vertices, left, y, front, r, g, b);
		addVertex(vertices, left, y, back, r, g, b);
		addVertex(vertices, right, y, front, r, g, b);
		addVertex(vertices, right, y, back, r, g, b);

		indices.push_back(start + 0);
		indices.push_back(start + 1);
		indices.push_back(start + 2);

		indices.push_back(start + 2);
		indices.push_back(start + 1);
		indices.push_back(start + 3);
	}

	void buildQuantumBoxMesh(
		std::vector<float>& vertices,
		std::vector<unsigned int>& indices,
		int mode,
		float excitedMix,
		float phase,
		bool isCharging,
		float releaseFlash,
		bool showMeasurement,
		float measurementU,
		float measurementV)
	{
		vertices.clear();
		indices.clear();
		vertices.reserve(SampleCount * SampleCount * 6 + 12 * 4 * 6);
		indices.reserve((SampleCount - 1) * (SampleCount - 1) * 6 + 12 * 6);

		for (unsigned int zIndex = 0; zIndex < SampleCount; ++zIndex)
		{
			const float v = static_cast<float>(zIndex) / static_cast<float>(SampleCount - 1);
			const float z = ZMin + BoxWidth * v;

			for (unsigned int xIndex = 0; xIndex < SampleCount; ++xIndex)
			{
				const float u = static_cast<float>(xIndex) / static_cast<float>(SampleCount - 1);
				const float x = XMin + BoxWidth * u;
				const float probability = evaluateProbability(u, v, mode, excitedMix, phase);
				const float y = BaseY + DensityScale * probability;

				const float r = 0.08f + 0.72f * probability + releaseFlash;
				const float g = 0.18f + 0.38f * probability + (isCharging ? 0.32f : 0.0f);
				const float b = 0.55f + 0.45f * probability;

				addVertex(vertices, x, y, z, r, g, b);
			}
		}

		for (unsigned int zIndex = 0; zIndex < SampleCount - 1; ++zIndex)
		{
			for (unsigned int xIndex = 0; xIndex < SampleCount - 1; ++xIndex)
			{
				const unsigned int p00 = zIndex * SampleCount + xIndex;
				const unsigned int p10 = p00 + 1;
				const unsigned int p01 = p00 + SampleCount;
				const unsigned int p11 = p01 + 1;

				indices.push_back(p00);
				indices.push_back(p01);
				indices.push_back(p10);

				indices.push_back(p10);
				indices.push_back(p01);
				indices.push_back(p11);
			}
		}

		addFloorRect(vertices, indices, XMin - 0.07f, XMin + 0.07f, ZMin, ZMax, BaseY - 0.03f, 1.0f, 0.25f, 0.25f);
		addFloorRect(vertices, indices, XMax - 0.07f, XMax + 0.07f, ZMin, ZMax, BaseY - 0.03f, 1.0f, 0.25f, 0.25f);
		addFloorRect(vertices, indices, XMin, XMax, ZMin - 0.07f, ZMin + 0.07f, BaseY - 0.03f, 1.0f, 0.25f, 0.25f);
		addFloorRect(vertices, indices, XMin, XMax, ZMax - 0.07f, ZMax + 0.07f, BaseY - 0.03f, 1.0f, 0.25f, 0.25f);

		addFloorRect(vertices, indices, XMin, XMax, ZMax + 0.35f, ZMax + 0.48f, BaseY - 0.55f, 0.12f, 0.12f, 0.16f);
		addFloorRect(vertices, indices, XMin, XMin + BoxWidth * excitedMix, ZMax + 0.35f, ZMax + 0.48f, BaseY - 0.43f, isCharging ? 0.2f : 0.8f, isCharging ? 0.95f : 0.45f, 1.0f);

		const float markerX = XMin + BoxWidth * (static_cast<float>(mode) / static_cast<float>(ModeCount - 1));
		addFloorRect(vertices, indices, markerX - 0.08f, markerX + 0.08f, ZMax + 0.62f, ZMax + 0.85f, BaseY - 0.08f, 1.0f, isCharging ? 0.95f : 0.55f, 0.2f);

		if (showMeasurement)
		{
			const float x = XMin + BoxWidth * measurementU;
			const float z = ZMin + BoxWidth * measurementV;
			const float y = BaseY + DensityScale * evaluateProbability(measurementU, measurementV, mode, excitedMix, phase) + 0.08f;
			addFloorRect(vertices, indices, x - 0.26f, x + 0.26f, z - 0.035f, z + 0.035f, y, 1.0f, 0.96f, 0.15f);
			addFloorRect(vertices, indices, x - 0.035f, x + 0.035f, z - 0.26f, z + 0.26f, y, 1.0f, 0.96f, 0.15f);
		}
	}

	void sampleMeasurement(float& outU, float& outV, int mode, float excitedMix, float phase)
	{
		static std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);

		for (int attempt = 0; attempt < 512; ++attempt)
		{
			const float u = unit(rng);
			const float v = unit(rng);
			const float probability = evaluateProbability(u, v, mode, excitedMix, phase);

			if (unit(rng) * 2.0f <= probability)
			{
				outU = u;
				outV = v;
				return;
			}
		}

		outU = 0.5f;
		outV = 0.5f;
	}
}

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
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	buildQuantumBoxMesh(
		vertices,
		indices,
		m_selectedMode,
		m_excitedMix,
		m_phaseTime,
		false,
		0.0f,
		false,
		m_measurementU,
		m_measurementV);

	VertexAttribute graphAttributes[] = {
		3,
		3
	};

	m_polygonVAO = m_graphicsEngine->createVertexArrayObject({
		vertices.data(),
		indices.data(),
		sizeof(float) * 6,
		static_cast<unsigned int>(vertices.size() / 6),
		static_cast<unsigned int>(indices.size()),
		graphAttributes,
		2
	});

	m_uniformBuffer = m_graphicsEngine->createUniformBuffer({
		sizeof(UniformData)
	});

	m_shaderProgram = m_graphicsEngine->createShaderProgram({
		L"Assets/Shaders/BasicShader.vert",
		L"Assets/Shaders/BasicShader.frag"
	});

	m_shaderProgram->setUniformBufferSlot("UniformData", 0);

	auto windowSize = m_display->getInnerSize();
	float aspect = 1.0f;

	if (windowSize.height != 0)
	{
		aspect = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
	}

	m_projectionMatrix = std::make_unique<Mat4>();
	m_viewMatrix = std::make_unique<Mat4>();
	m_worldMatrix = std::make_unique<Mat4>();
	m_trans = std::make_unique<Mat4>();

	float fovRadians = 45.0f * (Pi / 180.0f);
	m_projectionMatrix->setPerspective(fovRadians, aspect, 0.01f, 100.0f);

	m_viewMatrix->setLookAt(
		Vec4(0.0f, 3.4f, 8.5f, 1.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f),
		Vec4(0.0f, 1.0f, 0.0f, 1.0f)
	);

	m_display->setTitle(L"Quantum Particle in a 2D Box");
}

void Game::onUpdate(const InputState& input)
{
	m_currentTime = std::chrono::high_resolution_clock::now();

	if (m_previousTime.time_since_epoch().count())
	{
		m_elapsedSeconds = m_currentTime - m_previousTime;
	}

	m_previousTime = m_currentTime;

	float deltaTime = static_cast<float>(m_elapsedSeconds.count());
	deltaTime = std::min(deltaTime, 0.05f);

	if (input.wasKeyPressed(VK_ESCAPE))
	{
		Quit();
		return;
	}

	if (input.wasKeyPressed('P'))
	{
		m_isPaused = !m_isPaused;
	}

	if (input.wasKeyPressed('R'))
	{
		m_selectedMode = 0;
		m_excitedMix = 0.45f;
		m_previewMix = m_excitedMix;
		m_phaseTime = 0.0f;
		m_animationSpeed = 1.0f;
		m_releaseFlash = 0.0f;
		m_isCharging = false;
		m_isPaused = false;
		m_showMeasurement = false;
	}

	if (input.wasKeyPressed(VK_SPACE) || input.wasKeyPressed(VK_RIGHT))
	{
		m_selectedMode = (m_selectedMode + 1) % ModeCount;
		m_releaseFlash = 0.25f;
	}

	if (input.wasKeyPressed(VK_LEFT))
	{
		m_selectedMode = (m_selectedMode + ModeCount - 1) % ModeCount;
		m_releaseFlash = 0.25f;
	}

	if (input.isKeyDown(VK_UP))
	{
		m_excitedMix = std::min(0.95f, m_excitedMix + deltaTime * 0.45f);
		m_previewMix = m_excitedMix;
	}

	if (input.isKeyDown(VK_DOWN))
	{
		m_excitedMix = std::max(0.0f, m_excitedMix - deltaTime * 0.45f);
		m_previewMix = m_excitedMix;
	}

	if (input.wasKeyPressed(VK_OEM_PLUS) || input.wasKeyPressed(VK_ADD))
	{
		m_animationSpeed = std::min(3.0f, m_animationSpeed + 0.25f);
	}

	if (input.wasKeyPressed(VK_OEM_MINUS) || input.wasKeyPressed(VK_SUBTRACT))
	{
		m_animationSpeed = std::max(0.0f, m_animationSpeed - 0.25f);
	}

	if (input.leftMousePressed)
	{
		m_isCharging = true;
		m_previewMix = 0.08f;
		m_selectedMode = (m_selectedMode + 1) % ModeCount;
	}

	if (input.leftMouseDown && m_isCharging)
	{
		m_previewMix = std::min(0.95f, m_previewMix + deltaTime * 0.55f);
	}

	if (input.leftMouseReleased && m_isCharging)
	{
		m_excitedMix = m_previewMix;
		m_isCharging = false;
		m_releaseFlash = 0.35f;
	}

	const int visibleMode = m_selectedMode;
	const float visibleMix = m_isCharging ? m_previewMix : m_excitedMix;

	if (input.rightMousePressed || input.wasKeyPressed('M'))
	{
		sampleMeasurement(m_measurementU, m_measurementV, visibleMode, visibleMix, m_phaseTime);
		m_showMeasurement = true;
		m_releaseFlash = 0.35f;
	}

	if (!m_isPaused)
	{
		const int nx = ModeNx[visibleMode];
		const int nz = ModeNz[visibleMode];
		const float energyGround = 0.5f * static_cast<float>(1 * 1 + 1 * 1) * Pi * Pi;
		const float energyExcited = 0.5f * static_cast<float>(nx * nx + nz * nz) * Pi * Pi;
		const float omega = energyExcited - energyGround;
		m_phaseTime += omega * deltaTime * 0.16f * m_animationSpeed;
	}

	if (m_releaseFlash > 0.0f)
	{
		m_releaseFlash = std::max(0.0f, m_releaseFlash - deltaTime);
	}

	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	buildQuantumBoxMesh(
		vertices,
		indices,
		visibleMode,
		visibleMix,
		m_phaseTime,
		m_isCharging,
		m_releaseFlash,
		m_showMeasurement,
		m_measurementU,
		m_measurementV);

	VertexAttribute graphAttributes[] = {
		3,
		3
	};

	m_polygonVAO = m_graphicsEngine->createVertexArrayObject({
		vertices.data(),
		indices.data(),
		sizeof(float) * 6,
		static_cast<unsigned int>(vertices.size() / 6),
		static_cast<unsigned int>(indices.size()),
		graphAttributes,
		2
	});

	auto windowSize = m_display->getInnerSize();
	m_graphicsEngine->setViewport(windowSize);

	float aspect = 1.0f;

	if (windowSize.height != 0)
	{
		aspect = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
	}

	m_projectionMatrix->setPerspective(45.0f * (Pi / 180.0f), aspect, 0.01f, 100.0f);
	m_worldMatrix->setIdentity();

	UniformData data = { *m_worldMatrix, *m_viewMatrix, *m_projectionMatrix };
	m_uniformBuffer->setData(&data);

	m_graphicsEngine->clear(Vec4(0.015f, 0.018f, 0.03f, 1.0f));

	m_graphicsEngine->setVertexArrayObject(m_polygonVAO);
	m_graphicsEngine->setUniformBuffer(m_uniformBuffer, 0);
	m_graphicsEngine->setShaderProgram(m_shaderProgram);
	m_graphicsEngine->drawTriangles(
		TriangleType::TriangleList,
		static_cast<unsigned int>(indices.size()),
		0
	);

	m_display->present(true);

	wchar_t title[256] = {};
	swprintf_s(
		title,
		L"Quantum 2D Box | mode (%d,%d) | mix %.2f | speed %.2fx | %s | LMB charge, RMB/M measure, arrows tune, P pause, R reset",
		ModeNx[visibleMode],
		ModeNz[visibleMode],
		visibleMix,
		m_animationSpeed,
		m_isPaused ? L"paused" : L"running");
	m_display->setTitle(title);
}


void Game::onQuit()
{
}

void Game::Run()
{
	onCreate();
	MSG msg = {};
	InputState input;

	while(m_isRunning)
	{
		input.beginFrame();

		while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
				case WM_QUIT:
				{
					m_isRunning = false;
					break;
				}
				case WM_LBUTTONDOWN:
				{
					if (!input.leftMouseDown)
					{
						input.leftMousePressed = true;
					}

					input.leftMouseDown = true;
					break;
				}
				case WM_LBUTTONUP:
				{
					input.leftMouseDown = false;
					input.leftMouseReleased = true;
					break;
				}
				case WM_RBUTTONDOWN:
				{
					if (!input.rightMouseDown)
					{
						input.rightMousePressed = true;
					}

					input.rightMouseDown = true;
					break;
				}
				case WM_RBUTTONUP:
				{
					input.rightMouseDown = false;
					input.rightMouseReleased = true;
					break;
				}
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
				{
					if (msg.wParam < 256)
					{
						if (!input.keysDown[msg.wParam])
						{
							input.keysPressed[msg.wParam] = true;
						}

						input.keysDown[msg.wParam] = true;
					}
					break;
				}
				case WM_KEYUP:
				case WM_SYSKEYUP:
				{
					if (msg.wParam < 256)
					{
						input.keysDown[msg.wParam] = false;
						input.keysReleased[msg.wParam] = true;
					}
					break;
				}
				default: 
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					break;
				}
			}
		}

		if (!m_isRunning)
		{
			break;
		}
		
		onUpdate(input);
	}

	onQuit();
}

void Game::Quit()
{
	m_isRunning = false;
}
