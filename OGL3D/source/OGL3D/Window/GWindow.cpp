#include <OGL3D/Window/GWindow.h>
#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <windows.h>
#include <assert.h>

//extern HINSTANCE hInstance;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
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

    RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU, FALSE);

    m_handle = CreateWindowExW(0, MAKEINTATOM(classID), L"OpenGL Window", WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, (rect.right-rect.left), (rect.bottom - rect.top), nullptr, nullptr, wcex.hInstance, nullptr);

    assert(m_handle);

	SetWindowLongPtrW((HWND)m_handle, GWLP_USERDATA, (LONG_PTR)this);

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

	glEnable(GL_DEPTH_TEST); //need to move to Game::Game()
    glDepthFunc(GL_LEQUAL); //need to move to Game::Game()
}

GWindow::~GWindow()
{
    if(m_deviceContextHandle)
    {
        ReleaseDC((HWND)m_handle, HDC(m_deviceContextHandle));
	}

    wglDeleteContext(HGLRC(m_deviceContext)); // can this come before ReleaseDC?
    DestroyWindow((HWND)m_handle);
}

Rect GWindow::getInnerSize()
{
	RECT rc = {};
    GetClientRect(HWND(m_handle), &rc);
	return Rect(rc.right - rc.left, rc.bottom - rc.top);
}

void GWindow::makeCurrentContext()
{
	wglMakeCurrent(HDC(m_deviceContextHandle), HGLRC(m_deviceContext));
}

void GWindow::present(bool vsync)
{
    wglSwapIntervalEXT(vsync);
	wglSwapLayerBuffers(HDC(m_deviceContextHandle), WGL_SWAP_MAIN_PLANE);
}