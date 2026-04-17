#pragma once

#include <OGL3D/Math/Rect.h>

// Needs to be set at CMake level
#ifndef UNICODE
#define UNICODE
#endif

class GWindow
{
public:
	GWindow();
	~GWindow();

	Rect getInnerSize();

	void makeCurrentContext();
	void present(bool vsync);

private:
	void* m_handle = nullptr;
	void* m_deviceContext = nullptr;
	void* m_deviceContextHandle = nullptr;
};

