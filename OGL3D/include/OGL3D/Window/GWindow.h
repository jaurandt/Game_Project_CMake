#pragma once

#include <OGL3D/Math/Rect.h>
#include <string>
#include <vector>

enum class InterfaceAction { Pause = 1, Reset, PreviousMode, NextMode, LessMix, MoreMix, Slower, Faster, Measure };

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
	void setTitle(const wchar_t* title);
	void updateInspector(const std::wstring& state, const std::wstring& measurement, bool paused);
	std::vector<InterfaceAction> takeActions();
	bool isSceneWindow(void* handle) const { return handle == m_handle; }
	void queueAction(InterfaceAction action) { m_actions.push_back(action); }

private:
	void* m_handle = nullptr;
	void* m_deviceContext = nullptr;
	void* m_deviceContextHandle = nullptr;
	void* m_panel = nullptr;
	void* m_stateLabel = nullptr;
	void* m_measurementLabel = nullptr;
	void* m_pauseButton = nullptr;
	void* m_font = nullptr;
	std::vector<InterfaceAction> m_actions;
};

