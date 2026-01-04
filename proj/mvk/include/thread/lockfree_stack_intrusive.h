
#ifndef _MVK_THREAD_LOCK_FREE_STACK_INTRUSIVE_
#define _MVK_THREAD_LOCK_FREE_STACK_INTRUSIVE_

#include <atomic>

namespace mvk
{
	namespace thread
	{
		template<typename T>
		class LockFreeStackIntrusive
		{
		public:
			struct Node
			{
				Node() {}

				virtual ~Node() {}

				T& get()
				{
					return *static_cast<T*>(this);
				}

				const T& get() const
				{
					return *static_cast<T*>(this);
				}

				Node& operator=(const Node& o)
				{
					return *this;
				}

			private:
				friend class LockFreeStackIntrusive;

				std::atomic<T*> next = nullptr;
			};

		public:
			LockFreeStackIntrusive() {}

			void push(T* node)
			{
				while (true)
				{
					T* old = top_.load(std::memory_order_relaxed);
					node->next.store(old, std::memory_order_relaxed);
					if (top_.compare_exchange_weak(
						old,
						node,
						std::memory_order_relaxed,
						std::memory_order_relaxed))
					{
						break;
					}
				}
			}

		private:
			std::atomic<T*> top_ = nullptr;
		};
	}
}

#endif
