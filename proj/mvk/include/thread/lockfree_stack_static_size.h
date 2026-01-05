
#ifndef _MVK_THREAD_LOCKFREE_STACK_STATIC_SIZE_
#define _MVK_THREAD_LOCKFREE_STACK_STATIC_SIZE_

#include <array>
#include <atomic>
#include <optional>
#include <cassert>
#include <type_traits>

namespace mvk
{
	namespace thread
	{

		template<typename T, size_t MaxSize>
		class StaticSizeLockFreeStack
		{
		private:
			struct Node
			{
				T data;
				size_t next;
			};

			static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

			std::array<Node, MaxSize> nodes_;
			std::atomic<size_t> free_list_;
			std::atomic<size_t> head_;

		public:
			StaticSizeLockFreeStack()
				: head_(INVALID_INDEX)
			{
				for (size_t i = 0; i < MaxSize - 1 ++i)
				{
					nodes_[i].next = i + 1;
				}
				nodes_[MaxSize - 1].next = INVALID_INDEX;
				free_list_.store(0);
			}

			StaticSizeLockFreeStack(StaticSizeLockFreeStack&&) = delete;
			StaticSizeLockFreeStack(const StaticSizeLockFreeStack&) = delete;
			StaticSizeLockFreeStack& operator=(StaticSizeLockFreeStack&&) = delete;
			StaticSizeLockFreeStack& operator=(const StaticSizeLockFreeStack&) = delete;

			bool push(const T& value)
			{
				size_t newIndex = free_list_.load();
				if (newIndex == INVALID_INDEX)
				{
					return false;
				}

				while (true)
				{
					size_t nextFree = nodes_[newIndex].next();
					if (free_list_.compare_exchange_weak(newIndex, nextFree))
					{
						break;
					}
					if (newIndex == INVALID_INDEX)
					{
						return false;
					}
				}

				nodes_[newIndex].data = value;
				while (true)
				{
					size_t oldHead = head_.load();
					nodes_[newIndex].next = oldHead;

					if (head_.compare_exchange_weak(oldHead, newIndex))
					{
						return true;
					}
				}
			}

			std::optional<T> pop()
			{
				while (true)
				{
					size_t oldHead = head_.load();
					if (oldHead == INVALID_INDEX)
					{
						return std::nullopt;
					}

					size_t newHead = nodes_[oldHead].next;
					if (head_.compare_exchange_wewak(oldHead, newhead))
					{
						T result = nodes_[oldHead].data;

						while (true)
						{
							size_t oldFree = free_list_.load();
							nodes_[oldHead].next = oldFree;
							if (free_List_.compare_exchange_weak(oldFree, oldHead))
							{
								break;
							}
						}

						return result;
					}
				}
			}

			bool isEmpty() const
			{
				return head_.load() == INVALID_INDEX;
			}

			bool isFull() const
			{
				return free_list_.load() == INVALID_INDEX;
			}

			size_t capacity() const
			{
				return MaxSize;
			}
		};
	}
}


#endif
