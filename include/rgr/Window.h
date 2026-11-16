#ifndef RGR_WINDOW_HEADER
#define RGR_WINDOW_HEADER

#include <rgr/Core.h>
#include <string>

namespace rgr {
	class EventSystem;
	class Window;

	struct WindowCreateInfo {
		EventSystem*	  eventSystem = nullptr;
		Vec2<int>		  size		  = { 0, 0 };
		std::wstring_view title		  = L"";
	}; // struct WindowCreateInfo

	class RenderTarget {
	public:
		RenderTarget(Window* window) : m_window(window) {}

		void Clear(Color color);
		void Present();

		void DrawText(Vec2<float> pos, Vec2<float> size, std::wstring_view string, Color color, float fontSize);
		void DrawLine(Vec2<float> from, Vec2<float> to, Color color, float lineWidth);
		void DrawTriangle(Vec2<float> p1, Vec2<float> p2, Vec2<float> p3, Color fillColor, Color outlineColor);
		void DrawRectangle(Vec2<float> pos, Vec2<float> size, Color fillColor, Color outlineColor);
		void DrawRoundedRectangle(Vec2<float> pos, Vec2<float> size, float radius, Color fillColor, Color outlineColor);

	private:
		Window* m_window = nullptr;
	}; // class RenderTarget

	class Window {
	public:
		friend class RenderTarget;
		struct Impl;

	public:
		Window(const WindowCreateInfo& info);
		Window(const Window&) = delete;
		Window(Window&&) noexcept = delete;
		~Window();

		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) noexcept = delete;

	public:
		inline RenderTarget GetRenderTarget() { return RenderTarget(this); }
		Viewport GetViewport() const;
		void PollEvents();

	private:
		class _BackBuffer {
		public:
			struct Impl;

		public:
			_BackBuffer(Window* window);
			_BackBuffer(const _BackBuffer&) = delete;
			_BackBuffer(_BackBuffer&&) noexcept = delete;
			~_BackBuffer();

			_BackBuffer& operator=(const _BackBuffer&) = delete;
			_BackBuffer& operator=(_BackBuffer&&) noexcept = delete;

			inline Impl* GetImpl() const { return m_impl; }

		private:
			Impl* m_impl = nullptr;
		}; // class BackBuffer

	private:
		Impl* m_impl = nullptr;
	}; // class Window

	struct WindowEvent {
	public:
		WindowEvent(Window* window) : window(window) {}
		Window* window;
	}; // class WindowEvent

	struct WindowCloseEvent : WindowEvent {
	public:
		WindowCloseEvent(Window* window) : WindowEvent(window) {}
	}; // struct WindowCloseEvent

	struct WindowMouseButtonEvent : WindowEvent {
	public:
		enum MouseButton : uint8_t { Left, Right };
		WindowMouseButtonEvent(Window* window, Vec2<float> pos, MouseButton button, bool isPressed) :
			WindowEvent(window), button(button), isPressed(isPressed), pos(pos) {}

	public:
		bool		isPressed;
		MouseButton button;
		Vec2<float> pos;
	}; // struct WindowMouseButtonEvent

	enum class Key {
		Unknown,
		Backspace, Enter
	}; // enum class Key

	struct WindowKeyEvent : WindowEvent {
	public:
		WindowKeyEvent(Window* window, Key key, bool isPressed) : WindowEvent(window), isPressed(isPressed), key(key) {}
		bool isPressed;
		Key key;
	}; // WindowKeyCharEvent

	struct WindowKeyCharEvent : WindowEvent {
	public:
		WindowKeyCharEvent(Window* window, wchar_t c) : WindowEvent(window), c(c) {}
		wchar_t c;
	}; // WindowKeyCharEvent
} // namespace rgr
#endif // !RGR_WINDOW_HEADER