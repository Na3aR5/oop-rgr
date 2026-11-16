#ifndef RGR_REGISTRATION_TEMPLATE_HEADER
#define RGR_REGISTRATION_TEMPLATE_HEADER

#include <string>
#include <vector>
#include <map>

namespace rgr {
	class RegistrationTemplate {
	private:
		struct _Input {
			std::string name;
			std::string written;
		}; // struct _Input

	public:
		class Panel {
		public:
			void AddInput(uint32_t id, const std::string& name);
			uint32_t GetInputID(uint32_t index) const;
			const std::string& GetInputName(uint32_t id) const;
			uint32_t GetInputCount() const;

		private:
			friend class RegistrationTemplate;
			Panel(RegistrationTemplate* regTemplate, uint32_t panelID) :
				m_panelID(panelID), m_template(regTemplate) {}

		private:
			uint32_t m_panelID;
			RegistrationTemplate* m_template;
		}; // class Panel

	public:
		Panel AddPanel();

	private:
		struct _Inputs {
		public:
			std::map<uint32_t, _Input> byID;
			std::vector<std::map<uint32_t, _Input>::iterator> byOrder;
		}; // struct _Inputs

		std::vector<_Inputs> m_panels;
	}; // class InfoInputPanel
} // namespace rgr
#endif // !RGR_REGISTRATION_TEMPLATE_HEADER