#ifndef RGR_GRAPHICS_DEVICE_HEADER
#define RGR_GRAPHICS_DEVICE_HEADER

namespace rgr {
	class GraphicsDevice {
	public:
		struct Impl;

	public:
		GraphicsDevice();
		~GraphicsDevice();

	private:
		Impl* m_impl = nullptr;
	}; // class GraphicsDevice
} // namespace rgr
#endif // !RGR_GRAPHICS_DEVICE_HEADER