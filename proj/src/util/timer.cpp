
#include "timer.h"

void Timer::update()
{
	end_ = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = end_ - start_;

	deltaTime_ = elapsed.count();
	fps_ = 1.0f / deltaTime_;

	start_ = std::chrono::steady_clock::now();
}