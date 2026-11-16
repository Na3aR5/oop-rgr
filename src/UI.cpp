#include <rgr/UI.h>
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cwctype>

#include <iostream>

#undef DrawText

namespace rgr {
	static constexpr uint32_t g_InputPadding = 30;

	static std::wstring ToWide(const std::string& str) {
		int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		std::wstring wstr(size, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), size);
		wstr.pop_back();
		return wstr;
	}

	static std::string ToUTF8(std::wstring_view str) {
		if (str.empty()) return {}; 
		int size = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
		if (size == 0) return {};
		std::string result(size, '\0');
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), size, nullptr, nullptr);
		return result;
	}

	void UIInput::OnLeftClick() {
		m_gainedFocus = true;
	}

	void UIInput::OnNotHit() {
		m_gainedFocus = false;
	}

	void UIInput::OnKeyPressed(Key key) {
		if (m_gainedFocus && key == Key::Backspace) {
			if (!m_inputString.empty())
				m_inputString.pop_back();
		}
		else if (key == Key::Enter) m_gainedFocus = false;
	}

	void UIInput::OnKeyChar(wchar_t c) {
		if (m_gainedFocus && std::iswprint(c)) {
			m_inputString += c;
		}
	}

	void UIInput::Draw(RenderTarget& renderTarget) {
		renderTarget.DrawText(m_viewport.pos, m_viewport.size, m_inputString, Color(255, 255, 255), m_viewport.size.y / 2.f);
	}

	void UIRectangle::Draw(RenderTarget& renderTarget, const Viewport& viewport) {
		renderTarget.DrawRectangle(m_pos + (Vec2<float>)viewport.pos, m_size, m_color, m_outlineColor);
	}

	bool UIRectangle::HitTest(Vec2<float> pos) const {
		return pos.x >= m_pos.x && pos.x + m_pos.x <= m_size.x &&
			   pos.y >= m_pos.y && pos.y + m_pos.y <= m_size.y;
	}

	void UIRoundedRectangle::Draw(RenderTarget& renderTarget, const Viewport& viewport) {
		renderTarget.DrawRoundedRectangle(m_pos + (Vec2<float>)viewport.pos, m_size, m_radius, m_color, m_outlineColor);
	}

	bool UIRoundedRectangle::HitTest(Vec2<float> pos) const {
		if (pos.x < m_pos.x || pos.x > m_pos.x + m_size.x ||
			pos.y < m_pos.y || pos.y > m_pos.y + m_size.y) {
			return false;
		}
		float r = std::min(m_radius, std::min(m_size.x, m_size.y) * 0.5f);

		float cx = std::clamp(pos.x, m_pos.x + r, m_pos.x + m_size.x - r);
		float cy = std::clamp(pos.y, m_pos.y + r, m_pos.y + m_size.y - r);

		float dx = pos.x - cx;
		float dy = pos.y - cy;

		return dx * dx + dy * dy <= r * r;
	}

	void UI::_Element::DrawChilds(RenderTarget& renderTarget) {
		for (std::unique_ptr<_Element>& child : this->childs) {
			if (child->shape != nullptr) {
				child->shape->Draw(renderTarget, child->viewport);
			}
			if (child->text != nullptr) {
				renderTarget.DrawText(
					child->text->viewport.pos + child->viewport.pos,
					child->text->viewport.size,
					child->text->string,
					child->text->color,
					child->text->fontSize
				);
			}
			if (child->widget != nullptr) {
				child->widget->Draw(renderTarget);
			}
			child->DrawChilds(renderTarget);
		}
	}

	UI::UI(Viewport viewport, EventSystem* eventSystem) : m_eventSystem(eventSystem) {
		m_root.viewport = viewport;
	}

	void UI::Draw(RenderTarget& renderTarget) {
		m_root.DrawChilds(renderTarget);
	}

	UI::Element UI::AddPanel(Element parent, const RegistrationTemplate::Panel& panel) {
		_Element* parentElement = parent.m_element;
		if (parentElement == nullptr) {
			parentElement = &m_root;
		}
		parentElement->childs.emplace_back(std::make_unique<_Element>());
		std::unique_ptr<_Element>& panelElement = parentElement->childs.back();

		panelElement->parent = parentElement;
		panelElement->viewport.size.x = static_cast<int>(0.95f * parentElement->viewport.size.x);
		panelElement->viewport.size.y = static_cast<int>(0.95f * parentElement->viewport.size.y);
		panelElement->viewport.pos.x = (parentElement->viewport.size.x - panelElement->viewport.size.x) / 2 + parentElement->viewport.pos.x;
		panelElement->viewport.pos.y = (parentElement->viewport.size.y - panelElement->viewport.size.y) / 2 + parentElement->viewport.pos.y;

		parentElement = panelElement.get();
		uint32_t inputCount = panel.GetInputCount();

		for (uint32_t i = 0; i < inputCount; ++i) {
			parentElement->childs.emplace_back(std::make_unique<_Element>());
			std::unique_ptr<_Element>& input = parentElement->childs.back();

			input->parent = parentElement;
			input->viewport.size.x = static_cast<int>(0.6 * parentElement->viewport.size.x);
			input->viewport.size.y = 50;
			input->viewport.pos.x = (parentElement->viewport.size.x - input->viewport.size.x) / 2 + parentElement->viewport.pos.x;
			input->viewport.pos.y = static_cast<int>(50 * (i + 1) + g_InputPadding * i) + parentElement->viewport.pos.y;

			int halfViewportSizeY = input->viewport.size.y / 2;

			input->shape = std::make_unique<UIRoundedRectangle>(
				Vec2<float>(0.f, 0.f + halfViewportSizeY),
				Vec2<float>((float)input->viewport.size.x, (float)halfViewportSizeY),
				5.f,
				Color(0, 120, 100, 255),
				Color(0, 80, 60)
			);

			input->text = std::make_unique<_Element::Text>();
			input->text->viewport.pos = { 0, 0 };
			input->text->viewport.size = Vec2<float>((float)input->viewport.size.x, (float)halfViewportSizeY);
			input->text->color = Color(255, 255, 255);
			input->text->string = ToWide(panel.GetInputName(panel.GetInputID(i)));
			input->text->fontSize = 10.f;

			input->widget = std::make_unique<UIInput>(Viewport{
				.pos = input->viewport.pos + input->shape->GetViewport().pos,
				.size = input->shape->GetViewport().size
			});

			input->mouseButtonListener = m_eventSystem->StartListen<WindowMouseButtonEvent>(input.get(),
				[](void* context, const WindowMouseButtonEvent& e) {
					_Element* element = (_Element*)context;
					if (element->shape->HitTest(e.pos - (Vec2<float>)element->viewport.pos)) {
						element->widget->OnLeftClick();
					}
					else {
						element->widget->OnNotHit();
					}
				}
			);
			input->keyListener = m_eventSystem->StartListen<WindowKeyEvent>(input.get(),
				[](void* context, const WindowKeyEvent& e) {
					_Element* element = (_Element*)context;
					if (e.isPressed) {
						element->widget->OnKeyPressed(e.key);
					}
				}
			);
			input->keyCharListener = m_eventSystem->StartListen<WindowKeyCharEvent>(input.get(),
				[](void* context, const WindowKeyCharEvent& e) {
					_Element* element = (_Element*)context;
					element->widget->OnKeyChar(e.c);
				}
			);
		}

		return Element(panelElement.get());
	}

	void UIArrow::Draw(RenderTarget& renderTarget, const Viewport& viewport) {
		Vec2<float> vec = m_to - m_pos;
		float vecLen = vec.Length();
		Vec2<float> pivot = m_to - vec * m_heightScale;
		Vec2<float> normal = (vec * (1.f / vecLen)).Normal();

		Vec2<float> secondPt = pivot + normal * m_width;
		Vec2<float> thirdPt = pivot - normal * m_width;

		Vec2<float> offset = (Vec2<float>)viewport.pos;

		renderTarget.DrawLine(m_pos + offset, m_to + offset, m_color, m_lineWidth);
		renderTarget.DrawTriangle(m_to + offset, secondPt + offset, thirdPt + offset, m_color, m_outlineColor);
	}

	Viewport UIArrow::GetViewport() const noexcept {
		Viewport viewport;
		viewport.pos = { (int)std::min(m_pos.x, m_to.x), (int)std::min(m_pos.y, m_to.y) };
		viewport.size = { (int)std::abs(m_pos.x - m_to.x), (int)std::abs(m_pos.y - m_to.y) };
		return viewport;
	}
}