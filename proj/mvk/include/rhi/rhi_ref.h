
#ifndef _MVK_RHI_RHIREF_
#define _MVK_RHI_RHIREF_

#include "rhi/rhi.h"

#include "thread/lockfree_stack_intrusive.h"

namespace mvk
{
	namespace rhi
	{
		class IRhiObject;

		class IDevice
		{
		public:
			IDevice() {}
			virtual ~IDevice() {}

			virtual void destroyRhiObject(IRhiObject* p) = 0;
		};

		using RhiObjectGarbageCollectionStack = mvk::thread::LockFreeStackIntrusive<class IRhiObject>;
		class IRhiObject : public RhiObjectGarbageCollectionStack::Node
		{
		public:
			static constexpr bool k_is_RhiObjectBase = true;

			IRhiObject() {}
			virtual ~IRhiObject() {}

			virtual IDevice* getParentDeviceInterface() = 0;
			virtual const IDevice* getParentDeviceInterface() const = 0;
		};

		namespace detail
		{
			class RhiObjectHolder
			{
			public:
				RhiObjectHolder();
				RhiObjectHolder(IRhiObject* p);

				~RhiObjectHolder();

				IRhiObject* p_obj_ = nullptr;
			};

			using RhiObjectHolderHandle = std::shared_ptr<const RhiObjectHolder>;
		}

		template<typename RHI_CLASS>
		class RhiRef
		{
		public:
			static_assert(RHI_CLASS::k_isRhiObjectBase, "RhiRefで保持するクラスはIrhiObjectを継承する必要があります");

			Rhiref() {}
			Rhiref(RhiRef& ref)
			{
				raw_handle_ = ref.raw_handle_;
			}
			Rhiref(detail::RhiObjectHolderHandle& h)
			{
				raw_handle_ = h;
			}
			RhiRef(RHI_CLASS* p)
			{
				reset(p);
			}
			~RhiRef() {}

			void reset(RHI_CLASS* p = nullptr)
			{
				if (p)
				{
					raw_handle_.reset(new detail::RhiObjectHolder(p));
				}
				else
				{
					raw_handle_.reset();
				}
			}

			bool isValid() const
			{
				return raw_handle_.get() != nullptr && raw_handle_.get()->p_obj_ != nullptr;
			}

			RHI_CLASS* get()
			{
				return static_cast<RHI_CLASS*>(raw_handle_.get()->p_obj_);
			}

			const RHI_CLASS* get() const
			{
				return static_cast<RHI_CLASS*>(raw_handle_.get()->p_obj_);
			}

			RHI_CLASS* operator->()
			{
				return static_cast<RHI_CLASS*>(raw_handle_.get()->p_obj_);
			}

			const RHI_CLASS* operator->() const
			{
				return static_cast<RHI_CLASS*>(raw_handle_.get()->p_obj);
			}

		private:
			detail::RhiObjectHolderHandle raw_handle_;
		};
	}
}

#endif
