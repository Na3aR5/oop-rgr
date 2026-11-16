#ifndef RGR_EVENT_SYSTEM_HEADER
#define RGR_EVENT_SYSTEM_HEADER

#include <rgr/HandleSet.h>

namespace rgr {
	class EventSystem {
	public:
		struct ListenerData {
			bool  mutableEvent = true;
			void* handler = nullptr;
			void* context = nullptr;
		}; // struct ListenerData

		using ListenerStorage = HandleSet<ListenerData, HandleSetRemoveStrategy::ArrayDefault>;

		template <typename Event>
		class EventListener {
		public:
			EventListener() = default;

		private:
			using _ListenerHandle = typename EventSystem::ListenerStorage::ValueHandle;

			friend class EventSystem;
			EventListener(_ListenerHandle listener) : m_listener(listener) {}

		private:
			_ListenerHandle m_listener;
		}; // class EventListener

	public:
		template <typename Event>
		void Dispatch(const Event& event) const {
			uint32_t eventID = _GetEventID<Event>();
			if (eventID >= m_eventListeners.size()) {
				return;
			}
			using OriginHandler = void(*)(void*, const Event&);
			for (ListenerData listenerData : m_eventListeners[eventID]) {
				((OriginHandler)listenerData.handler)(listenerData.context, event);
			}
		}

		template <typename Event>
		EventListener<Event> StartListen(void* context, void(*handler)(void*, const Event&)) {
			uint32_t eventID = _GetEventID<Event>();
			if (eventID >= m_eventListeners.size()) {
				m_eventListeners.resize((size_t)eventID + 1);
			}
			auto handle = m_eventListeners[eventID].Emplace(ListenerData{
				.handler = (void*)handler,
				.context = context
			});
			return EventListener<Event>(handle);
		}

		template <typename Event>
		void StopListen(EventListener<Event> listener) {
			uint32_t eventID = _GetEventID<Event>();
			if (eventID >= m_eventListeners.size() || !m_eventListeners[eventID].IsValid(listener.m_listener)) {
				return;
			}
			m_eventListeners[eventID].Remove(listener.m_listener);
		}

	private:
		inline static uint32_t s_EventTypeCounter = 0;

		template <typename Event>
		struct _EventIDStruct {
			inline static uint32_t s_ID = s_EventTypeCounter++;
		}; // struct _EventIDStruct

		template <typename Event>
		static uint32_t _GetEventID() {
			return _EventIDStruct<Event>::s_ID;
		}

	private:
		std::vector<ListenerStorage> m_eventListeners;
	}; // class EventSystem
} // namespace rgr
#endif // !RGR_EVENT_SYSTEM_HEADER