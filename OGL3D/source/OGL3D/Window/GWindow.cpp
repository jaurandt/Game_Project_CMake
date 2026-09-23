#include <OGL3D/Window/GWindow.h>
#include <windows.h>
#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <assert.h>
#include <algorithm>

namespace
{
    constexpr int PanelWidth = 340;
    LRESULT CALLBACK InspectorProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
        {
            auto window = reinterpret_cast<GWindow*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
            if (window) window->queueAction(static_cast<InterfaceAction>(LOWORD(wParam)));
            SetFocus(GetParent(handle));
            return 0;
        }
        if (message == WM_CTLCOLORSTATIC)
        {
            SetTextColor(reinterpret_cast<HDC>(wParam), RGB(219, 231, 242));
            SetBkColor(reinterpret_cast<HDC>(wParam), RGB(23, 32, 46));
            SetDCBrushColor(reinterpret_cast<HDC>(wParam), RGB(23, 32, 46));
            return reinterpret_cast<LRESULT>(GetStockObject(DC_BRUSH));
        }
        if (message == WM_ERASEBKGND)
        {
            RECT rect;
            GetClientRect(handle, &rect);
            SetDCBrushColor(reinterpret_cast<HDC>(wParam), RGB(23, 32, 46));
            FillRect(reinterpret_cast<HDC>(wParam), &rect, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
            return 1;
        }
        return DefWindowProcW(handle, message, wParam, lParam);
    }
}

//extern HINSTANCE hInstance;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_GETMINMAXINFO:
        {
            auto limits = reinterpret_cast<MINMAXINFO*>(lParam);
            limits->ptMinTrackSize = { 1000, 820 };
            return 0;
        }
        case WM_SIZE:
        {
            if (auto panel = GetDlgItem(hWnd, 100))
                MoveWindow(panel, 0, 0, PanelWidth, HIWORD(lParam), TRUE);
            return 0;
        }
        case WM_DESTROY:
        {
            GWindow* window = (GWindow*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            break;
        }
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

GWindow::GWindow()
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //***CS_OWNDC recommended for OpenGL***(ADDED)***
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    //wcex.hInstance = hInstance;
    //wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = /*(HBRUSH)(COLOR_WINDOW + 1);*/ (HBRUSH)GetStockObject(NULL_BRUSH); //***No background for OpenGL***(ADDED)***
    //wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_APPLICATION);
	wcex.lpszClassName = L"OpenGLWindowClass";
    //wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    auto classID = RegisterClassExW(&wcex);
    assert(classID);

    RECT rect = { 0, 0, 1340, 800 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU, FALSE);

    m_handle = CreateWindowExW(0, MAKEINTATOM(classID), L"OpenGL Window", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, (rect.right-rect.left), (rect.bottom - rect.top), nullptr, nullptr, wcex.hInstance, nullptr);

    assert(m_handle);

	SetWindowLongPtrW((HWND)m_handle, GWLP_USERDATA, (LONG_PTR)this);

    WNDCLASSW panelClass = {};
    panelClass.lpfnWndProc = InspectorProc;
    panelClass.lpszClassName = L"QuantumInspector";
    panelClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&panelClass);
    m_panel = CreateWindowW(panelClass.lpszClassName, L"Simulation inspector",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, PanelWidth, 800,
        HWND(m_handle), reinterpret_cast<HMENU>(100), nullptr, nullptr);
    SetWindowLongPtrW(HWND(m_panel), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    m_font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    auto label = [&](const wchar_t* text, int y, int height) {
        HWND control = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE,
            20, y, 300, height, HWND(m_panel), nullptr, nullptr, nullptr);
        SendMessageW(control, WM_SETFONT, WPARAM(m_font), TRUE);
        return control;
    };
    auto button = [&](const wchar_t* text, int x, int y, int width, InterfaceAction action) {
        HWND control = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            x, y, width, 30, HWND(m_panel), reinterpret_cast<HMENU>(static_cast<INT_PTR>(action)), nullptr, nullptr);
        SendMessageW(control, WM_SETFONT, WPARAM(m_font), TRUE);
        return control;
    };
    label(L"QUANTUM EXPLORER", 20, 24);
    label(L"Particle in a 2D infinite square well", 48, 24);
    m_pauseButton = button(L"Pause  [P]", 20, 86, 144, InterfaceAction::Pause);
    button(L"Reset  [R]", 176, 86, 144, InterfaceAction::Reset);
    m_stateLabel = label(L"", 134, 170);
    button(L"Previous mode", 20, 312, 144, InterfaceAction::PreviousMode);
    button(L"Next mode", 176, 312, 144, InterfaceAction::NextMode);
    button(L"Mix -", 20, 350, 144, InterfaceAction::LessMix);
    button(L"Mix +", 176, 350, 144, InterfaceAction::MoreMix);
    button(L"Slower", 20, 388, 144, InterfaceAction::Slower);
    button(L"Faster", 176, 388, 144, InterfaceAction::Faster);
    button(L"Sample position  [M]", 20, 434, 300, InterfaceAction::Measure);
    m_measurementLabel = label(L"No position sampled yet.", 474, 54);
    label(L"READING THE SURFACE", 542, 24);
    label(L"Height and brightness show relative\nprobability density, not a particle's path.\nRed edges mark the confining walls.\nYellow cross marks the latest sample.\n\nMix is the excited-state amplitude; its\nsquare gives the excited-state probability.\nSamples do not collapse this simulation.", 572, 152);
    label(L"Arrows: mode / mix   +/-: speed\nHold left mouse in scene: charge mix", 730, 44);

	ShowWindow((HWND)m_handle, SW_SHOW);
	UpdateWindow((HWND)m_handle);

    //Creating OpenGL Rendering Context

	auto hDC = GetDC(HWND(m_handle));
    m_deviceContextHandle = hDC;

    int pixelFormatAttributes[] =     {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
	};

	int pixelFormat = 0;
	UINT numFormats = 0;
    wglChoosePixelFormatARB(hDC, pixelFormatAttributes, nullptr, 1, &pixelFormat, &numFormats);

    assert(numFormats);

	PIXELFORMATDESCRIPTOR pfd = {};
	DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	SetPixelFormat(hDC, pixelFormat, &pfd);

    int openGLContextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
	};

	m_deviceContext = wglCreateContextAttribsARB(hDC, 0, openGLContextAttributes);
	assert(m_deviceContext);
}

GWindow::~GWindow()
{
    if(m_deviceContextHandle)
    {
        ReleaseDC((HWND)m_handle, HDC(m_deviceContextHandle));
	}

    wglDeleteContext(HGLRC(m_deviceContext)); // can this come before ReleaseDC?
    DestroyWindow((HWND)m_handle);
    DeleteObject(m_font);
}

Rect GWindow::getInnerSize()
{
	RECT rc = {};
    GetClientRect(HWND(m_handle), &rc);
	return Rect(PanelWidth, 0, (std::max)(1L, rc.right - PanelWidth), (std::max)(1L, rc.bottom));
}

void GWindow::makeCurrentContext()
{
	wglMakeCurrent(HDC(m_deviceContextHandle), HGLRC(m_deviceContext));

    glEnable(GL_DEPTH_TEST); //moved from GWindow::GWindow()
    glDepthFunc(GL_LEQUAL);  //moved from GWindow::GWindow()

    glEnable(GL_CULL_FACE); // moved from drawTriangles, should be set once in the engine
	glCullFace(GL_BACK); // moved from drawTriangles, should be set once in the engine
}

void GWindow::present(bool vsync)
{
    wglSwapIntervalEXT(vsync);
	wglSwapLayerBuffers(HDC(m_deviceContextHandle), WGL_SWAP_MAIN_PLANE);
}

void GWindow::setTitle(const wchar_t* title)
{
    SetWindowTextW(HWND(m_handle), title);
}

void GWindow::updateInspector(const std::wstring& state, const std::wstring& measurement, bool paused)
{
    auto update = [](HWND control, const std::wstring& text) {
        wchar_t previous[1024] = {};
        GetWindowTextW(control, previous, 1024);
        if (text != previous) SetWindowTextW(control, text.c_str());
    };
    update(HWND(m_stateLabel), state);
    update(HWND(m_measurementLabel), measurement);
    update(HWND(m_pauseButton), paused ? L"Resume  [P]" : L"Pause  [P]");
}

std::vector<InterfaceAction> GWindow::takeActions()
{
    std::vector<InterfaceAction> actions;
    actions.swap(m_actions);
    return actions;
}
