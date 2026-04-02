#pragma once

#include <OGL3D/Graphics/VertexArrayObject.h>
#include <OGL3D/Math/Mat4.h>
#include <memory>
#include <chrono>

class GraphicsEngine;
class GWindow;

class Game
{
public:
	Game();
	~Game();

	virtual void onCreate();
	virtual void onUpdate(InputMouse mouse);
	virtual void onQuit();

	void Run();
	void Quit();

protected:
	bool m_isRunning = true;
	
	std::unique_ptr<GraphicsEngine> m_graphicsEngine;
	std::unique_ptr<GWindow> m_display;

	std::chrono::high_resolution_clock::time_point m_currentTime;
	std::chrono::high_resolution_clock::time_point m_previousTime;
	std::chrono::duration<double> m_elapsedSeconds;
	float m_scale = 0.0f;
	float m_sinScale = 0.0f;
	float m_cosScale = 0.0f;

	std::unique_ptr<Mat4> m_projectionMatrix;
	std::unique_ptr<Mat4> m_viewMatrix;
	std::unique_ptr<Mat4> m_worldMatrix;
	std::unique_ptr<Mat4> m_trans;

	VertexArrayObjectPtr m_polygonVAO;
	UniformBufferPtr m_uniformBuffer;
	ShaderProgramPtr m_shaderProgram;
};
