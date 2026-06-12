#include <OGL3D/Game/Game.h>
#include <OGL3D/Math/Vec4.h>
#include <OGL3D/Window/GWindow.h>
#include <OGL3D/Graphics/GraphicsEngine.h>
#include <OGL3D/Graphics/ShaderProgram.h>
#include <OGL3D/Graphics/UniformBuffer.h>
#include <OGL3D/Math/Mat4.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>

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
	constexpr unsigned int sampleCount = 48;
	constexpr float pi = 3.1415927f;
	constexpr float boxWidth = 6.0f;
	constexpr float xMin = -boxWidth * 0.5f;
	constexpr float xMax = boxWidth * 0.5f;
	constexpr float zMin = -boxWidth * 0.5f;
	constexpr float zMax = boxWidth * 0.5f;
	constexpr float baseY = -1.35f;
	constexpr float densityScale = 0.95f;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(sampleCount * sampleCount * 6);
	indices.reserve((sampleCount - 1) * (sampleCount - 1) * 6);

	auto addVertex = [&vertices](float x, float y, float z, float r, float g, float b)
	{
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(r);
		vertices.push_back(g);
		vertices.push_back(b);
	};

	auto addFloorRect = [&vertices, &indices, &addVertex](float left, float right, float front, float back, float y, float r, float g, float b)
	{
		unsigned int start = static_cast<unsigned int>(vertices.size() / 6);

		addVertex(left,  y, front, r, g, b);
		addVertex(left,  y, back,  r, g, b);
		addVertex(right, y, front, r, g, b);
		addVertex(right, y, back,  r, g, b);

		indices.push_back(start + 0);
		indices.push_back(start + 1);
		indices.push_back(start + 2);

		indices.push_back(start + 2);
		indices.push_back(start + 1);
		indices.push_back(start + 3);
	};

	float c2 = 0.45f;
	float c1 = sqrtf(1.0f - c2 * c2);

	for (unsigned int zIndex = 0; zIndex < sampleCount; ++zIndex)
	{
		float v = static_cast<float>(zIndex) / static_cast<float>(sampleCount - 1);
		float z = zMin + boxWidth * v;

		for (unsigned int xIndex = 0; xIndex < sampleCount; ++xIndex)
		{
			float u = static_cast<float>(xIndex) / static_cast<float>(sampleCount - 1);
			float x = xMin + boxWidth * u;

			float groundState = sinf(pi * u) * sinf(pi * v);
			float excitedState = sinf(2.0f * pi * u) * sinf(pi * v);

			float probability = c1 * c1 * groundState * groundState +
				c2 * c2 * excitedState * excitedState +
				2.0f * c1 * c2 * groundState * excitedState;

			float y = baseY + densityScale * probability;

			float r = 0.1f + 0.85f * probability;
			float g = 0.22f + 0.45f * probability;
			float b = 0.75f + 0.25f * probability;

			addVertex(x, y, z, r, g, b);
		}
	}

	for (unsigned int zIndex = 0; zIndex < sampleCount - 1; ++zIndex)
	{
		for (unsigned int xIndex = 0; xIndex < sampleCount - 1; ++xIndex)
		{
			unsigned int p00 = zIndex * sampleCount + xIndex;
			unsigned int p10 = p00 + 1;
			unsigned int p01 = p00 + sampleCount;
			unsigned int p11 = p01 + 1;

			indices.push_back(p00);
			indices.push_back(p01);
			indices.push_back(p10);

			indices.push_back(p10);
			indices.push_back(p01);
			indices.push_back(p11);
		}
	}

	addFloorRect(xMin - 0.07f, xMin + 0.07f, zMin, zMax, baseY - 0.03f, 1.0f, 0.25f, 0.25f);
	addFloorRect(xMax - 0.07f, xMax + 0.07f, zMin, zMax, baseY - 0.03f, 1.0f, 0.25f, 0.25f);
	addFloorRect(xMin, xMax, zMin - 0.07f, zMin + 0.07f, baseY - 0.03f, 1.0f, 0.25f, 0.25f);
	addFloorRect(xMin, xMax, zMax - 0.07f, zMax + 0.07f, baseY - 0.03f, 1.0f, 0.25f, 0.25f);

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

	float fovRadians = 45.0f * (pi / 180.0f);
	m_projectionMatrix->setPerspective(fovRadians, aspect, 0.01f, 100.0f);

	m_viewMatrix->setLookAt(
		Vec4(0.0f, 3.4f, 8.5f, 1.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f),
		Vec4(0.0f, 1.0f, 0.0f, 1.0f)
	);
}

void Game::onUpdate(const InputState& input)
{
	constexpr unsigned int sampleCount = 48;
	constexpr float pi = 3.1415927f;
	constexpr float boxWidth = 6.0f;
	constexpr float xMin = -boxWidth * 0.5f;
	constexpr float xMax = boxWidth * 0.5f;
	constexpr float zMin = -boxWidth * 0.5f;
	constexpr float zMax = boxWidth * 0.5f;
	constexpr float baseY = -1.35f;
	constexpr float densityScale = 0.95f;

	m_currentTime = std::chrono::high_resolution_clock::now();

	if (m_previousTime.time_since_epoch().count())
	{
		m_elapsedSeconds = m_currentTime - m_previousTime;
	}

	m_previousTime = m_currentTime;

	float deltaTime = static_cast<float>(m_elapsedSeconds.count());
	m_scale += deltaTime;

	constexpr int modeCount = 7;
	constexpr int modeNx[modeCount] = { 2, 1, 2, 3, 1, 3, 4 };
	constexpr int modeNz[modeCount] = { 1, 2, 2, 1, 3, 3, 2 };

	static int committedMode = 0;
	static int previewMode = 0;
	static bool isCharging = false;
	static float chargedMix = 0.45f;
	static float previewMix = 0.45f;
	static float releaseFlash = 0.0f;

	if (input.leftMousePressed)
	{
		isCharging = true;
		previewMix = 0.08f;
		previewMode = committedMode + 1;

		if (previewMode >= modeCount)
		{
			previewMode = 0;
		}
	}

	if (input.leftMouseDown && isCharging)
	{
		previewMix += deltaTime * 0.45f;

		if (previewMix > 0.92f)
		{
			previewMix = 0.92f;
		}
	}

	if (input.leftMouseReleased && isCharging)
	{
		committedMode = previewMode;
		chargedMix = previewMix;
		isCharging = false;
		releaseFlash = 0.35f;
	}

	if (input.rightMousePressed)
	{
		committedMode = 0;
		previewMode = 0;
		chargedMix = 0.45f;
		previewMix = chargedMix;
		isCharging = false;
		releaseFlash = 0.0f;
	}

	if (releaseFlash > 0.0f)
	{
		releaseFlash -= deltaTime;

		if (releaseFlash < 0.0f)
		{
			releaseFlash = 0.0f;
		}
	}

	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(sampleCount * sampleCount * 6 + 8 * 4 * 6);
	indices.reserve((sampleCount - 1) * (sampleCount - 1) * 6 + 8 * 6);

	auto addVertex = [&vertices](float x, float y, float z, float r, float g, float b)
	{
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(r);
		vertices.push_back(g);
		vertices.push_back(b);
	};

	auto addFloorRect = [&vertices, &indices, &addVertex](float left, float right, float front, float back, float y, float r, float g, float b)
	{
		unsigned int start = static_cast<unsigned int>(vertices.size() / 6);

		addVertex(left,  y, front, r, g, b);
		addVertex(left,  y, back,  r, g, b);
		addVertex(right, y, front, r, g, b);
		addVertex(right, y, back,  r, g, b);

		indices.push_back(start + 0);
		indices.push_back(start + 1);
		indices.push_back(start + 2);

		indices.push_back(start + 2);
		indices.push_back(start + 1);
		indices.push_back(start + 3);
	};

	int visibleMode = isCharging ? previewMode : committedMode;
	int excitedNx = modeNx[visibleMode];
	int excitedNz = modeNz[visibleMode];
	float c2 = isCharging ? previewMix : chargedMix;
	float c1 = sqrtf(1.0f - c2 * c2);

	float energyGround = 0.5f * static_cast<float>(1 * 1 + 1 * 1) * pi * pi;
	float energyExcited = 0.5f * static_cast<float>(excitedNx * excitedNx + excitedNz * excitedNz) * pi * pi;
	float omega = energyExcited - energyGround;

	float timePhase = omega * m_scale * 0.16f;

	for (unsigned int zIndex = 0; zIndex < sampleCount; ++zIndex)
	{
		float v = static_cast<float>(zIndex) / static_cast<float>(sampleCount - 1);
		float z = zMin + boxWidth * v;

		for (unsigned int xIndex = 0; xIndex < sampleCount; ++xIndex)
		{
			float u = static_cast<float>(xIndex) / static_cast<float>(sampleCount - 1);
			float x = xMin + boxWidth * u;

			float groundState = sinf(pi * u) * sinf(pi * v);
			float excitedState = sinf(static_cast<float>(excitedNx) * pi * u) *
				sinf(static_cast<float>(excitedNz) * pi * v);

			float probability = c1 * c1 * groundState * groundState +
				c2 * c2 * excitedState * excitedState +
				2.0f * c1 * c2 * groundState * excitedState * cosf(timePhase);

			float y = baseY + densityScale * probability;

			float r = 0.08f + 0.72f * probability + releaseFlash;
			float g = 0.18f + 0.38f * probability + (isCharging ? 0.32f : 0.0f);
			float b = 0.55f + 0.45f * probability;

			addVertex(x, y, z, r, g, b);
		}
	}

	for (unsigned int zIndex = 0; zIndex < sampleCount - 1; ++zIndex)
	{
		for (unsigned int xIndex = 0; xIndex < sampleCount - 1; ++xIndex)
		{
			unsigned int p00 = zIndex * sampleCount + xIndex;
			unsigned int p10 = p00 + 1;
			unsigned int p01 = p00 + sampleCount;
			unsigned int p11 = p01 + 1;

			indices.push_back(p00);
			indices.push_back(p01);
			indices.push_back(p10);

			indices.push_back(p10);
			indices.push_back(p01);
			indices.push_back(p11);
		}
	}

	addFloorRect(xMin - 0.07f, xMin + 0.07f, zMin, zMax, baseY - 0.03f, 1.0f, 0.25f, 0.25f);
	addFloorRect(xMax - 0.07f, xMax + 0.07f, zMin, zMax, baseY - 0.03f, 1.0f, 0.25f, 0.25f);
	addFloorRect(xMin, xMax, zMin - 0.07f, zMin + 0.07f, baseY - 0.03f, 1.0f, 0.25f, 0.25f);
	addFloorRect(xMin, xMax, zMax - 0.07f, zMax + 0.07f, baseY - 0.03f, 1.0f, 0.25f, 0.25f);

	float meterLeft = xMin;
	float meterRight = xMin + boxWidth * c2;
	float meterBottom = baseY - 0.55f;
	float meterTop = baseY - 0.43f;
	addFloorRect(xMin, xMax, zMax + 0.35f, zMax + 0.48f, meterBottom, 0.12f, 0.12f, 0.16f);
	addFloorRect(meterLeft, meterRight, zMax + 0.35f, zMax + 0.48f, meterTop, isCharging ? 0.2f : 0.8f, isCharging ? 0.95f : 0.45f, 1.0f);

	float markerX = xMin + boxWidth * (static_cast<float>(visibleMode) / static_cast<float>(modeCount - 1));
	addFloorRect(markerX - 0.08f, markerX + 0.08f, zMax + 0.62f, zMax + 0.85f, baseY - 0.08f, 1.0f, isCharging ? 0.95f : 0.55f, 0.2f);

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
