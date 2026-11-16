#ifndef RGR_HANDLE_SET_HEADER
#define RGR_HANDLE_SET_HEADER

#include <vector>

namespace rgr {
	enum class HandleSetRemoveStrategy : uint8_t {
		SwapAndPop,
		ArrayDefault
	}; // enum HandleSetRemoveStrategy

	template <typename T, HandleSetRemoveStrategy RemoveStrategy = HandleSetRemoveStrategy::SwapAndPop>
	class HandleSet;

	namespace _datastructures {
		class HandleSetBase {
		public:
			static constexpr uint32_t s_MaxGeneration = (~(uint32_t)0);
			static constexpr uint32_t s_MaxHandleCount = (~(uint32_t)0) >> 1;

		public:
			template <typename T>
			class Handle {
			public:
				Handle() noexcept = default;
				inline bool operator==(const Handle& other) const noexcept { return m_id == other.m_id; }

				inline uint32_t GetID() const noexcept { return (uint32_t)(m_id & UINT32_MAX); }
				inline uint32_t GetGeneration() const noexcept { return (uint32_t)(m_id >> 32); }
				static constexpr Handle Null() noexcept { return Handle(UINT32_MAX, UINT32_MAX); }

				template <typename U>
				inline explicit operator Handle<U>() const noexcept { return Handle<U>(GetID(), GetGeneration()); }

			public:
				template <typename _T>
				friend class Handle;

				template <typename _T, HandleSetRemoveStrategy RemoveStrategy>
				friend class rgr::HandleSet;

			private:
				explicit constexpr Handle(uint32_t id, uint32_t gen) :
					m_id((uint64_t)id | ((uint64_t)gen << 32)) {}

				uint64_t m_id;
			}; // class Handle

			class Slot {
			public:
				Slot() noexcept = default;
				Slot(uint32_t index, uint32_t gen) noexcept :
					m_index((index << 1) | 1), m_gen(gen) {}

			public:
				inline uint32_t GetGeneration() const noexcept { return m_gen; }
				inline uint32_t GetIndex() const noexcept { return m_index >> 1; }

				inline void SetIndex(uint32_t index) noexcept { m_index = (index << 1) | 1; }
				inline void IncrementGeneration() noexcept { ++m_gen; }

				inline void Empty() noexcept { m_index &= ~(uint32_t)1; }
				inline void Reuse() noexcept { m_index |= 1; }
				inline bool IsEmpty() const noexcept { return (m_index & 1) == 0; }

			private:
				uint32_t m_index = 0;
				uint32_t m_gen = 0;
			}; // class Slot
		}; // class HandleSetBase
	} // namespace _datastructures

	template <typename T, HandleSetRemoveStrategy RemoveStrategy>
	class HandleSet : public _datastructures::HandleSetBase {
	public:
		static_assert(std::is_swappable_v<T> || std::is_abstract_v<T>, "aqua::HandleSet - Value type must be swappable");

	public:
		using ValueType = T;
		using ValueHandle = Handle<ValueType>;
		using Iterator = typename std::vector<T>::iterator;
		using ConstIterator = typename std::vector<T>::const_iterator;

	public:
		template <typename ... Types>
		ValueHandle Emplace(Types&& ... args) noexcept {
			uint32_t valueIndex = static_cast<uint32_t>(m_values.size());

			if (valueIndex < m_valueIndices.size()) {
				uint32_t valueID = m_indexToId[valueIndex];

				m_values.emplace_back(std::forward<Types>(args)...);
				m_valueIndices[valueID].Reuse();
				m_valueIndices[valueID].SetIndex(valueIndex);

				return ValueHandle(valueID, m_valueIndices[valueID].GetGeneration());
			}

			m_values.emplace_back(std::forward<Types>(args)...);
			m_valueIndices.emplace_back(valueIndex, 0);
			m_indexToId.emplace_back(valueIndex);

			return ValueHandle(valueIndex, 0);
		}

		bool IsValid(ValueHandle handle) const noexcept {
			uint32_t handleID = handle.GetID();
			return handleID < m_valueIndices.size() &&
				m_valueIndices[handleID].GetIndex() < m_values.size() &&
				!m_valueIndices[handleID].IsEmpty() &&
				m_valueIndices[handleID].GetGeneration() == handle.GetGeneration();
		}

		ValueType& Get(ValueHandle handle) noexcept { return m_values[m_valueIndices[handle.GetID()].GetIndex()]; }
		const ValueType& Get(ValueHandle handle) const noexcept { return m_values[m_valueIndices[handle.GetID()].GetIndex()]; }

		ValueHandle GetHandleByID(uint32_t id) const noexcept {
			const Slot& slot = m_valueIndices[id];
			return ValueHandle(slot.GetIndex(), slot.GetGeneration());
		}

		bool IsEmpty() const noexcept { return m_values.empty(); }
		size_t GetSize() const noexcept { return m_values.size(); }

		void Remove(ValueHandle handle) noexcept {
			if (!IsValid(handle)) {
				return;
			}
			if constexpr (RemoveStrategy == HandleSetRemoveStrategy::SwapAndPop) {
				_RemoveBySwapAndPop(handle);
			}
			else {
				_RemoveByArrayDefault(handle);
			}
		}

		void Clear() noexcept {
			m_values.clear();
			m_indexToId.clear();
			m_valueIndices.clear();
		}

	public:
		Iterator      begin()        noexcept { return m_values.begin(); }
		ConstIterator begin()  const noexcept { return m_values.begin(); }
		ConstIterator cbegin() const noexcept { return m_values.cbegin(); }

		Iterator      end()        noexcept { return m_values.end(); }
		ConstIterator end()  const noexcept { return m_values.end(); }
		ConstIterator cend() const noexcept { return m_values.cend(); }

	private:
		void _RemoveBySwapAndPop(ValueHandle handle) noexcept {
			uint32_t valueIndex = m_valueIndices[handle.GetID()].GetIndex();
			uint32_t lastValueID = m_indexToId[m_values.size() - 1];
			uint32_t currValueID = m_indexToId[valueIndex];

			Slot& lastValueSlot = m_valueIndices[lastValueID];
			Slot& currValueSlot = m_valueIndices[currValueID];

			std::swap(m_values[valueIndex], m_values.back());
			std::swap(m_indexToId[valueIndex], m_indexToId[m_values.size() - 1]);

			lastValueSlot.SetIndex(valueIndex);

			currValueSlot.IncrementGeneration();
			currValueSlot.Empty();
			m_values.pop();
		}

		void _RemoveByArrayDefault(ValueHandle handle) noexcept {
			uint32_t valueIndex = m_valueIndices[handle.GetID()].GetIndex();
			uint32_t valueCount = (uint32_t)m_values.size();

			uint32_t toRemoveID = m_indexToId[valueIndex];
			for (uint32_t i = valueIndex + 1; i < valueCount; ++i) {
				m_indexToId[i - 1] = m_indexToId[i];
				m_valueIndices[m_indexToId[i - 1]].SetIndex(i - 1);
			}
			m_indexToId.back() = toRemoveID;
			m_valueIndices[toRemoveID].SetIndex(static_cast<uint32_t>(m_indexToId.size() - 1));

			m_values.erase(m_values.begin() + valueIndex);
		}

	private:
		std::vector<T>		  m_values;
		std::vector<uint32_t> m_indexToId;
		std::vector<Slot>	  m_valueIndices;
	}; // class HandleSet
} // namespace rgr
#endif // !RGR_HANDLE_SET_HEADER