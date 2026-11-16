#include <rgr/GraphicsDevice.h>

#include <windows.h>
#include <gdiplus.h>

namespace rgr {
	struct GraphicsDevice::Impl {
	public:
		ULONG_PTR gdiplusToken = 0;
		Gdiplus::GdiplusStartupInput startupInput;
	}; // struct GraphicsDevice::Impl

	GraphicsDevice::GraphicsDevice() {
		m_impl = new Impl();
		Gdiplus::GdiplusStartup(&m_impl->gdiplusToken, &m_impl->startupInput, nullptr);
	}

	GraphicsDevice::~GraphicsDevice() {
		if (m_impl != nullptr) {
			Gdiplus::GdiplusShutdown(m_impl->gdiplusToken);
			delete m_impl;
		}
	}
} // namespace rgr