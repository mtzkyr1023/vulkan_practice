#ifndef _TIMER_H_
#define _TIMER_H_

#include "chrono"
#include "iostream"

class Timer
{
private:
	Timer() :
		start_(std::chrono::high_resolution_clock::now()),
		end_(std::chrono::high_resolution_clock::now()),
		targetFps_(60),
		deltaTime_(1.0f),
		fps_(60.0f)
	{}
	~Timer() = default;

public:
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;
	Timer(Timer&&) = delete;
	Timer& operator=(Timer&&) = delete;

	static Timer& instance()
	{
		static Timer timer;
		return timer;
	}


	void setFps(int fps) { targetFps_ = fps; }
	void update();

	float deltaTime() { return deltaTime_; }
	float fps() { return fps_; }

protected:
	std::chrono::steady_clock::time_point start_;
	std::chrono::steady_clock::time_point end_;

	int targetFps_;
	float deltaTime_;
	float fps_;
};

#endif