#include <rgr/Application.h>
#include <thread>

namespace rgr {
	Application::Application() {
		m_eventSystem = std::make_unique<EventSystem>();
		m_graphicsDevice = std::make_unique<GraphicsDevice>();
		m_mainWindow = std::make_unique<Window>(WindowCreateInfo{
			.eventSystem = m_eventSystem.get(),
			.size        = Vec2<int>(500, 720),
			.title       = L"ООП: Розрахунково-графічна робота",
		});
		m_ui = std::make_unique<UI>(m_mainWindow->GetViewport(), m_eventSystem.get());

		RegistrationTemplate::Panel panel = m_registrationTemplate.AddPanel();
		panel.AddInput(0, "Email");
		panel.AddInput(1, "Username");
		panel.AddInput(2, "Password");

		m_ui->AddPanel(UI::Element::Null(), panel);

		m_windowCloseListener = m_eventSystem->StartListen<WindowCloseEvent>(this,
			[](void* context, const WindowCloseEvent& e) {
				Application* app = (Application*)context;
				if (e.window == app->m_mainWindow.get()) {
					app->UnsetState(State::Started);
				}
			}
		);
	}

	Application::~Application() {
		m_eventSystem->StopListen(m_windowCloseListener);
	}

	void Application::Run() {
		if (!HasState(State::Started)) {
			SetState(State::Started);
		}

		while (HasState(State::Started)) {
			m_mainWindow->PollEvents();

			RenderTarget renderTarget = m_mainWindow->GetRenderTarget();
			renderTarget.Clear(Color(0, 150, 120));
				m_ui->Draw(renderTarget);
			renderTarget.Present();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
} // namespace rgr