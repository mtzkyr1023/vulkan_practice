
#ifndef _MVK_UTIL_NONCOPYABLE_
#define _MVK_UTIL_NONCOPYABLE_

namespace mvk
{
	namespace util
	{
		template<class T>
		class NonCopyable
		{
		protected:
			NonCopyable() {}
			virtual ~NonCopyable() {}
		
		private:
			NonCopyable(const NonCopyable&) {}
			NonCopyable& operator=(const NonCopyable&) {}
		};
	}
}

#endif