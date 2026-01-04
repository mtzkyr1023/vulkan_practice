
#define NOMINMAX
#include <Windows.h>

#include "boot/win/boot_application.win.h"

namespace mvk
{
	namespace boot
	{
		BootApplication* BootApplication::create()
		{
			return new BootApplicationDep();
		}

		void BootApplicationDep::run(ApplicationBase* app)
		{
			if (!app->initialize())
				return;

			MSG msg;
			while (true)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					if (!app->execute())
					{
						break;
					}
				}
			}
		}
	}
}
