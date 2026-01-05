
#ifndef _MVK_THREAD_LOCKFREE_STACK_FIXED_SIZE_
#define _MVK_THREAD_LOCKFREE_STACK_FIXED_SIZE_

#include <vector>
#include <atomic>
#include <optional>
#include <memory>
#include <cassert>
#include <type_traits>


namespace mvk
{
	namespace thread
	{
		template<typename T>
		class FixedSizeLockFreeStack
		{
		private:
			struct Node
			{
				T data;
				size_t next;
			};

			static constexpr size_t INVALID_INDEX = static_cast<size_t>(~0u);

			std::vector<Node> nodes_ = {};
			std::atomic<size_t> free_list_ = {};
			std::atomic<size_t> head_ = {};
			size_t capacity_ = {};
			bool initialized_ = {};

		public:
			FixedSizeLockFreeStack()
				: nodes_()
				, free_list_(INVALID_INDEX)
				, head_(INVALID_INDEX)
				, capacity_(0)
				, initialized_(false)
			{
			}

			bool initialize(size_t capacity)
			{
				if (initialized_)
				{
					return false;
				}

				capacity_ = capacity;
				nodes_.resize(capacity);

				for (size_t i = 0; i < capacity - 1; ++i)
				{
					nodes_[i].next = i + 1;
				}
				nodes_[capacity_ - 1].next = INVALID_INDEX;
				free_list_.store(0);
				head_.store(INVALID_INDEX);

				initialized_ = true;
				return true;
			}

			FixedSizeLockFreeStack(FixedSizeLockFreeStack&&) = delete;
			FixedSizeLockFreeStack(const FixedSizeLockFreeStack&) = delete;
			FixedSizeLockFreeStack& operator=(FixedSizeLockFreeStack&&) = delete;
			FixedSizeLockFreeStack& operator=(const FixedSizeLockFreeStack&) = delete;

			bool push(const T& value)
			{
				assert(initialized_ && "Stack must be initialized before use");

				size_t newIndex = free_list_.load();
				if (newIndex == INVALID_INDEX)
				{
					return false;
				}

				while (true)
				{
					size_t nextFree = nodes_[newIndex].next;
					if (free_list_.compare_exchange_weak(newIndex, nextFree))
					{
						break;
					}

					if (newIndex == INVALID_INDEX)
					{
						return false;
					}
				}

				nodes_[newIndex] = value;

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
				assert(initialized_ && "Stack must be initialize before use");

				while (true)
				{
					size_t oldHead = head_.load();
					if (oldHead == INVALID_INDEX)
					{
						return std::nullopt;
					}

					size_t newHead = nodes_[oldHead].next;
					if (head_.compare_exchange_weak(oldHead, newHead))
					{
						T result = nodes_[oldHead].next;

						while (true)
						{
							size_t oldFree = free_list_.load();
							nodes_[oldHead].next = oldFree;

							if (free_list_.compare_exchange_weak(oldFree, oldHead))
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
				assert(initialized_ && "Stak must be initialized before use");
				return head_.load() == INVALID_INDEX;
			}
			bool isFull() const
			{
				assert(initialized_ && "Stack must be initialized before use");
				return free_list_.load() == INVALID_INDEX;
			}

			size_t capacity() const
			{
				assert(initialized_ && "Stack must be initialized before use");
				return capacity_;
			}
		};
	}
}

#endif

