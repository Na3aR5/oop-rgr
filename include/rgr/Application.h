#ifndef RGR_APPLICATION_HEADER
#define RGR_APPLICATION_HEADER

#include <rgr/UI.h>
#include <rgr/EventSystem.h>
#include <rgr/GraphicsDevice.h>

namespace rgr {
	class Application {
	public:
		enum class State : uint64_t {
			Started = 0x1
		}; // enum class State

	public:
		Application();
		~Application();

	public:
		void Run();

		inline bool HasState(State state) const { return m_states & (std::underlying_type_t<State>)state; }
		inline void SetState(State state) { m_states |= (std::underlying_type_t<State>)state; }
		inline void UnsetState(State state) { m_states &= ~(std::underlying_type_t<State>)state; }

	private:
		std::underlying_type_t<State>				 m_states = 0;
		std::unique_ptr<EventSystem>			     m_eventSystem;
		std::unique_ptr<GraphicsDevice>				 m_graphicsDevice;
		std::unique_ptr<Window>						 m_mainWindow;
		std::unique_ptr<UI>							 m_ui;
		RegistrationTemplate						 m_registrationTemplate;
		EventSystem::EventListener<WindowCloseEvent> m_windowCloseListener;
	}; // class Application
} // namespace rgr
#endif // !RGR_APPLICATION_HEADER