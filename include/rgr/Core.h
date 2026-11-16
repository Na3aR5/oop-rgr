#ifndef RGR_CORE_HEADER
#define RGR_CORE_HEADER

#include <cmath>
#include <type_traits>

namespace rgr {
	template <typename T>
	struct Vec2 {
	public:
		Vec2() = default;
		Vec2(T x, T y) : x(x), y(y) {}

		Vec2 Normal() const { return Vec2(y, -x); }
		float Length() const { return std::hypot(x, y); }

		Vec2<float> Normalize() const {
			float invLength = 1.f / Length();
			return Vec2<float>((float)x * invLength, (float)y * invLength);
		}

		template <typename U> requires (!std::is_same_v<T, U>)
		operator Vec2<U>() const { return Vec2<U>(static_cast<U>(x), static_cast<U>(y)); }

	public:
		T x = static_cast<T>(0);
		T y = static_cast<T>(0);
	}; // struct Vec2

	struct Color {
	public:
		Color() = default;
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

	public:
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		uint8_t a = 255;
	}; // struct Color

	struct Viewport {
		Vec2<int> pos = { 0, 0 };
		Vec2<int> size = { 0, 0 };
	}; // struct Viewport

	template <typename T>
	Vec2<T> operator+(const Vec2<T>& l, const Vec2<T>& r) { return Vec2<T>(l.x + r.x, l.y + r.y); }

	template <typename T>
	Vec2<T> operator-(const Vec2<T>& l, const Vec2<T>& r) { return Vec2<T>(l.x - r.x, l.y - r.y); }

	template <typename T>
	Vec2<T> operator*(const Vec2<T>& l, T r) { return Vec2<T>(l.x * r, l.y * r); }
} // namespace rgr
#endif // !RGR_CORE_HEADER