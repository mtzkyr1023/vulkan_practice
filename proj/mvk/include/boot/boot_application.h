
#ifndef _MVK_BOOT_BOOT_APPLICATION_
#define _MVK_BOOT_BOOT_APPLICATION_

#include "util/noncopyable.h"

namespace mvk
{
	namespace boot
	{
		class ApplicationBase
		{
		public:
			virtual ~ApplicationBase() {}

			virtual bool initialize() = 0;

			virtual bool execute() = 0;
		};

		class BootApplication : public util::NonCopyable<BootApplication>
		{
		public:
			static BootApplication* create();

		public:
			BootApplication() {}

			virtual ~BootApplication() {}

			virtual void run(ApplicationBase* app) = 0;
		};
	}
}

#endif