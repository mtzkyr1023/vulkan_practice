
#ifndef _MVK_RHI_RHI_OBJECT_GARBAGE_COLLECT_
#define _MVK_RHI_RHI_OBJECT_GARBAGE_COLLECT_

#include <atomic>
#include <array>

#include "rhi/rhi_ref.h"

namespace mvk
{
	namespace rhi
	{
		class DeviceDep;



		class RhiObjectBase : public IRhiObject
		{
		public:
			RhiObjectBase() {}
			virtual ~RhiObjectBase() {}


			IDevice* getParentDeviceInterface() override;
			const IDevice* getParentDeviceInterface() const override;

			DeviceDep* getParentDevice();
			DeviceDep* getParentDevice() const;

		protected:
			void initializeRhiObject(DeviceDep* p_device);

		protected:
			DeviceDep* p_parent_device_ = nullptr;
		};


		class GarbageCollector
		{
		public:
			GarbageCollector();
			~GarbageCollector();

			void initialize();
			void finalize();

			void readyToNewFrame();

			void execute();

			void enqueue(IRhiObject* p_obj);

		private:
			std::atomic_int flip_index_ = 0;

			std::array<RhiObjectGarbageCollectionStack, 3> frame_stack_;
		};
	}
}

#endif
