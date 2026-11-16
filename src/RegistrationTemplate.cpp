#include <rgr/RegistrationTemplate.h>

namespace rgr {
	RegistrationTemplate::Panel rgr::RegistrationTemplate::AddPanel() {
		m_panels.emplace_back();
		return Panel(this, static_cast<uint32_t>(m_panels.size() - 1));
	}

	void RegistrationTemplate::Panel::AddInput(uint32_t inputID, const std::string& name) {
		RegistrationTemplate::_Input input;
		input.name = name;
		auto it = m_template->m_panels[m_panelID].byID.emplace(inputID, std::move(input)).first;
		m_template->m_panels[m_panelID].byOrder.emplace_back(it);
	}

	uint32_t RegistrationTemplate::Panel::GetInputID(uint32_t index) const {
		return m_template->m_panels[m_panelID].byOrder[index]->first;
	}

	const std::string& RegistrationTemplate::Panel::GetInputName(uint32_t id) const {
		return m_template->m_panels[m_panelID].byID[id].name;
	}

	uint32_t RegistrationTemplate::Panel::GetInputCount() const {
		return static_cast<uint32_t>(m_template->m_panels[m_panelID].byID.size());
	}
}