
#ifndef _MVK_RHI_RHIREF_
#define _MVK_RHI_RHIREF_


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

		using RhiObjectGarbageCollectionStack = int;
	}
}

#endif
