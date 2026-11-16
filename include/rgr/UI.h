#ifndef RGR_UI_HEADER
#define RGR_UI_HEADER

#include <rgr/Window.h>
#include <rgr/EventSystem.h>
#include <rgr/RegistrationTemplate.h>

#include <vector>
#include <memory>

namespace rgr {
	class IUIShape {
	public:
		IUIShape(Vec2<float> pos, Color color, Color outlineColor) :
			m_pos(pos), m_color(color), m_outlineColor(outlineColor) {}

		virtual void Draw(RenderTarget& renderTarget, const Viewport& viewport) = 0;
		virtual bool HitTest(Vec2<float> pos) const = 0;
		virtual Viewport GetViewport() const noexcept = 0;

	protected:
		Vec2<float> m_pos;
		Color       m_color;
		Color       m_outlineColor;
	}; // class IUIShape

	class IUIWidget {
	public:
		virtual void OnLeftClick() = 0;
		virtual void OnNotHit() = 0;
		virtual void OnKeyPressed(Key key) = 0;
		virtual void OnKeyChar(wchar_t c) = 0;
		virtual void Draw(RenderTarget& renderTarget) = 0;
	}; // class IUIWidget

	class UIRectangle final : public IUIShape {
	public:
		UIRectangle(Vec2<float> pos, Vec2<float> size, Color color, Color outlineColor) :
			IUIShape(pos, color, outlineColor), m_size(size) {}

		virtual void Draw(RenderTarget& renderTarget, const Viewport& viewport) override;
		virtual bool HitTest(Vec2<float> pos) const override;
		virtual Viewport GetViewport() const noexcept override { return { m_pos, m_size }; }

	private:
		Vec2<float> m_size;
	}; // class UIRectangle

	class UIRoundedRectangle final : public IUIShape {
	public:
		UIRoundedRectangle(Vec2<float> pos, Vec2<float> size, float radius, Color color, Color outlineColor) :
			IUIShape(pos, color, outlineColor), m_size(size), m_radius(radius) {}

		virtual void Draw(RenderTarget& renderTarget, const Viewport& viewport) override;
		virtual bool HitTest(Vec2<float> pos) const override;
		virtual Viewport GetViewport() const noexcept override { return { m_pos, m_size }; }

	private:
		float m_radius;
		Vec2<float> m_size;
	}; // class UIRoundedRectangle

	class UIArrow final : public IUIShape {
	public:
		UIArrow(Vec2<float> from, Vec2<float> to, float lineWidth, float width, float heightScale, Color color) :
			IUIShape(from, color, color), m_to(to), m_lineWidth(lineWidth), m_width(width), m_heightScale(heightScale) {}

		virtual void Draw(RenderTarget& renderTarget, const Viewport& viewport) override;
		virtual Viewport GetViewport() const noexcept override;

	private:
		float		m_lineWidth;
		float		m_width;
		float		m_heightScale;
		Vec2<float> m_to;
	}; // class UIArrow

	class UIInput : public IUIWidget {
	public:
		UIInput(const Viewport& inputViewport) noexcept : m_viewport(inputViewport) {}

		virtual void OnLeftClick() override;
		virtual void OnNotHit() override;
		virtual void OnKeyPressed(Key key) override;
		virtual void OnKeyChar(wchar_t c) override;
		virtual void Draw(RenderTarget& renderTarget) override;

	private:
		bool		 m_gainedFocus = false;
		Viewport	 m_viewport;
		std::wstring m_inputString;
	}; // class UIInput

	class UI {
	private:
		struct _Element {
		public:
			struct Text {
				float        fontSize = 0.f;
				Color        color    = { 0, 0, 0 };
				Viewport     viewport = {};
				std::wstring string;
			}; // struct Text

			void DrawChilds(RenderTarget& renderTarget);

		public:
			_Element*							   parent   = nullptr;
			std::unique_ptr<IUIShape>			   shape    = nullptr;
			std::unique_ptr<Text>				   text     = nullptr;
			std::unique_ptr<IUIWidget>			   widget   = nullptr;
			Viewport							   viewport = {};
			std::vector<std::unique_ptr<_Element>> childs;

			EventSystem::EventListener<WindowMouseButtonEvent> mouseButtonListener = {};
			EventSystem::EventListener<WindowKeyEvent>		   keyListener		   = {};
			EventSystem::EventListener<WindowKeyCharEvent>     keyCharListener     = {};
		}; // struct _Element

	public:
		class Element {
		public:
			inline static Element Null() { return Element(nullptr); }
			inline bool operator==(Element other) const { return m_element == other.m_element; }

		private:
			friend class UI;
			Element(_Element* element) : m_element(element) {};
			_Element* m_element = nullptr;
		}; // class Element

	public:
		UI(Viewport viewport, EventSystem* eventSystem);
		void Draw(RenderTarget& renderTarget);

		template <typename ShapeType, typename ... Types>
		requires (std::is_base_of_v<IUIShape, ShapeType>)
		Element AddElement(Element parent, Viewport viewport, Types&& ... shapeArgs) {
			_Element* parentElement = parent.m_element;
			if (parentElement == nullptr) {
				parentElement = &m_root;
			}
			parentElement->childs.emplace_back(std::make_unique<_Element>());
			std::unique_ptr<_Element>& emplaced = parentElement->childs.back();

			emplaced->viewport = viewport;
			emplaced->parent = parentElement;
			emplaced->shape = std::make_unique<ShapeType>(std::forward<Types>(shapeArgs)...);

			return Element(emplaced.get());
		}

		Element AddPanel(Element parent, const RegistrationTemplate::Panel& panel);

	private:
		EventSystem* m_eventSystem = nullptr;
		_Element     m_root		   = {};
	}; // class UI
} // namespace rgr
#endif // !RGR_UI_HEADER