#include <rgr/Window.h>
#include <rgr/EventSystem.h>

#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>

#include <memory>

#undef DrawText

namespace rgr {
	struct Window::Impl {
	public:
		Window*								 window      = nullptr;
		EventSystem*						 eventSystem = nullptr;
		HWND								 hwnd		 = NULL;
		std::unique_ptr<Window::_BackBuffer> backBuffer  = nullptr;
	}; // struct Window::Impl

	static LRESULT CALLBACK _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	Window::Window(const WindowCreateInfo& info) {
		const wchar_t WINDOW_CLASS_NAME[] = L"__rgrwindowclass__";

		WNDCLASSEXW windowClass = {};
		windowClass.cbSize        = sizeof(windowClass);
		windowClass.hInstance     = GetModuleHandleW(nullptr),
		windowClass.lpszClassName = WINDOW_CLASS_NAME;
		windowClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		windowClass.lpfnWndProc   = _WndProc;

		RegisterClassExW(&windowClass);

		m_impl = new Impl();

		HWND hwnd = CreateWindowExW(
			0,
			WINDOW_CLASS_NAME,
			info.title.data(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			info.size.x,
			info.size.y,
			NULL,
			NULL,
			GetModuleHandleW(nullptr),
			m_impl
		);

		m_impl->window = this;
		m_impl->eventSystem = info.eventSystem;
		m_impl->hwnd = hwnd;
		m_impl->backBuffer = std::make_unique<_BackBuffer>(this);

		ShowWindow(hwnd, SW_SHOW);
	}

	Window::~Window() {
		if (m_impl != nullptr) {
			DestroyWindow(m_impl->hwnd);

			delete m_impl;
		}
	}

	Viewport Window::GetViewport() const {
		RECT clientRect;
		GetClientRect(m_impl->hwnd, &clientRect);
		SIZE size = SIZE{ clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };
		return Viewport{ { 0, 0 }, { (int)size.cx, (int)size.cy } };
	}

	void Window::PollEvents() {
		MSG msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	struct Window::_BackBuffer::Impl {
	public:
		Window* window = nullptr;
		std::unique_ptr<Gdiplus::Bitmap> bitmap = nullptr;
		std::unique_ptr<Gdiplus::Graphics> graphics = nullptr;
	}; // struct Window::_BackBuffer::Impl

	Window::_BackBuffer::_BackBuffer(Window* window) {
		m_impl = new Impl();

		RECT clientRect;
		GetClientRect(window->m_impl->hwnd, &clientRect);
		SIZE size = SIZE{ clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };

		m_impl->window = window;
		m_impl->bitmap = std::make_unique<Gdiplus::Bitmap>(size.cx, size.cy, PixelFormat32bppARGB);
		m_impl->graphics = std::make_unique<Gdiplus::Graphics>(m_impl->bitmap.get());
		m_impl->graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	}

	Window::_BackBuffer::~_BackBuffer() {
		if (m_impl != nullptr) {
			delete m_impl;
		}
	}

	LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)	{
		if (msg == WM_NCCREATE) {
			CREATESTRUCTW* createStruct = (CREATESTRUCTW*)lParam;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		Window::Impl* impl = (Window::Impl*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

		switch (msg) {
			case WM_LBUTTONDOWN: {
				POINT pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				impl->eventSystem->Dispatch(WindowMouseButtonEvent(impl->window, Vec2<float>((float)pos.x, (float)pos.y), WindowMouseButtonEvent::Left, true));
				break;
			}

			case WM_KEYDOWN: {
				Key key = Key::Unknown;
				if (wParam == VK_RETURN) key = Key::Enter;
				else if (wParam == VK_BACK) key = Key::Backspace;
				impl->eventSystem->Dispatch(WindowKeyEvent(impl->window, key, true));
				break;
			}

			case WM_CHAR:
				impl->eventSystem->Dispatch(WindowKeyCharEvent(impl->window, static_cast<wchar_t>(wParam)));
				break;

			case WM_CLOSE:
				impl->eventSystem->Dispatch(WindowCloseEvent(impl->window));
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;
		}
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void RenderTarget::Clear(Color color) {
		m_window->m_impl->backBuffer->GetImpl()->graphics->Clear(
			Gdiplus::Color(color.r, color.g, color.b)
		);
	}

	void RenderTarget::Present() {
		HDC hdc = GetDC(m_window->m_impl->hwnd);
		Gdiplus::Graphics graphics{ hdc };

		RECT clientRect;
		GetClientRect(m_window->m_impl->hwnd, &clientRect);
		SIZE size = SIZE{ clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };

		graphics.DrawImage(m_window->m_impl->backBuffer->GetImpl()->bitmap.get(),
			0, 0, size.cx, size.cy
		);

		ReleaseDC(m_window->m_impl->hwnd, hdc);
	}

	void RenderTarget::DrawText(Vec2<float> pos, Vec2<float> size, std::wstring_view string, Color color, float fontSize) {
		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignmentNear);
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

		Gdiplus::RectF rect(pos.x, pos.y, size.x, size.y);
		Gdiplus::Font font(L"Arial", fontSize);
		Gdiplus::SolidBrush brush(Gdiplus::Color(color.a, color.r, color.g, color.b));

		m_window->m_impl->backBuffer->GetImpl()->graphics->DrawString(
			string.data(), static_cast<INT>(string.length()), &font, rect, &format, &brush
		);
	}

	void RenderTarget::DrawLine(Vec2<float> from, Vec2<float> to, Color color, float lineWidth) {
		if (color.a == 0) {
			return;
		}
		Gdiplus::Pen linePen(Gdiplus::Color(color.a, color.r, color.g, color.b), lineWidth);
		m_window->m_impl->backBuffer->GetImpl()->graphics->DrawLine(
			&linePen,
			Gdiplus::PointF(from.x, from.y),
			Gdiplus::PointF(to.x, to.y)
		);
	}

	void RenderTarget::DrawTriangle(Vec2<float> p1, Vec2<float> p2, Vec2<float> p3, Color fillColor, Color outlineColor) {
		Gdiplus::PointF points[3] = {
			Gdiplus::PointF(p1.x, p1.y),
			Gdiplus::PointF(p2.x, p2.y),
			Gdiplus::PointF(p3.x, p3.y),
		};
		if (fillColor.a != 0) {
			Gdiplus::SolidBrush fillBrush(Gdiplus::Color(fillColor.a, fillColor.r, fillColor.g, fillColor.b));
			m_window->m_impl->backBuffer->GetImpl()->graphics->FillPolygon(&fillBrush, points, 3);
		}
		if (outlineColor.a != 0) {
			Gdiplus::Pen outlinePen(Gdiplus::Color(outlineColor.a, outlineColor.r, outlineColor.g, outlineColor.b), 2.f);
			m_window->m_impl->backBuffer->GetImpl()->graphics->DrawPolygon(&outlinePen, points, 3);
		}
	}

	void RenderTarget::DrawRectangle(Vec2<float> pos, Vec2<float> size, Color fillColor, Color outlineColor) {
		if (fillColor.a != 0) {
			Gdiplus::SolidBrush fillBrush(Gdiplus::Color(fillColor.a, fillColor.r, fillColor.g, fillColor.b));
			m_window->m_impl->backBuffer->GetImpl()->graphics->FillRectangle(&fillBrush, pos.x, pos.y, size.x, size.y);
		}
		if (outlineColor.a != 0) {
			Gdiplus::Pen outlinePen(Gdiplus::Color(outlineColor.a, outlineColor.r, outlineColor.g, outlineColor.b), 2.f);
			m_window->m_impl->backBuffer->GetImpl()->graphics->DrawRectangle(&outlinePen, pos.x, pos.y, size.x, size.y);
		}
	}

	void RenderTarget::DrawRoundedRectangle(Vec2<float> pos, Vec2<float> size, float radius, Color fillColor, Color outlineColor) {
		Gdiplus::GraphicsPath gpath;

		float d = 2 * radius;
		float right = pos.x + size.x;
		float bottom = pos.y + size.y;

		gpath.AddArc(pos.x,     pos.y,   d, d,    180, 90);
		gpath.AddArc(right - d, pos.y,   d, d,    270, 90);
		gpath.AddArc(right - d, bottom - d, d, d, 0,   90);
		gpath.AddArc(pos.x,     bottom - d, d, d, 90,  90);

		gpath.CloseFigure();

		if (fillColor.a != 0) {
			Gdiplus::SolidBrush fillBrush(Gdiplus::Color(fillColor.a, fillColor.r, fillColor.g, fillColor.b));
			m_window->m_impl->backBuffer->GetImpl()->graphics->FillPath(&fillBrush, &gpath);
		}
		if (outlineColor.a != 0) {
			Gdiplus::Pen outlinePen(Gdiplus::Color(outlineColor.a, outlineColor.r, outlineColor.g, outlineColor.b), 2.f);
			m_window->m_impl->backBuffer->GetImpl()->graphics->DrawPath(&outlinePen, &gpath);
		}
	}
}

#ifdef UNICODE
#define DrawText  DrawTextW
#else
#define DrawText  DrawTextA
#endif // !UNICODE