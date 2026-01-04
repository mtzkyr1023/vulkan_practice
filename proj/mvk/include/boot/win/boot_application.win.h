
#ifndef _MVK_BOOT_BOOT_APPLICATION_WIN_
#define _MVK_BOOT_BOOT_APPLICATION_WIN_

#include "boot/boot_application.h"

namespace mvk
{
	namespace boot
	{
		class BootApplicationDep : public BootApplication
		{
		public:
			BootApplicationDep() {}
			~BootApplicationDep() {}

			void run(ApplicationBase* app) override;
		};
	}
}

#endif