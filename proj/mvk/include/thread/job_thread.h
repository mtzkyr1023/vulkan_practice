
#ifndef _MVK_THREAD_JOBTHREAD_
#define _MVK_THREAD_JOBTREAED_

#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

namespace mvk
{
	namespace thread
	{
		class SingletonJobThread
		{
		public:
			SingletonJobThread()
			{
				thread_instance_ = std::thread([&]() {execute(); });
			}
			~SingletonJobThread()
			{
				{
					std::unique_lock<std::mutex> lock(condition_mutex_);
					terminate_signal_ = true;

					condition_var_.notify_all();
				}

				thread_instance_.join();
			}

			bool begin(std::function<void(void)> func)
			{
				std::unique_lock<std::mutex> lock(condition_mutex_);

				if (job_signal_)
					return false;

				func_ = func;
				job_signal_ = true;

				condition_var_.notify_all();
				return true;
			}

			void wait()
			{
				std::unique_lock<std::mutex> lock(condition_mutex_);
				if (!job_signal_)
					return;

				condition_var_.wait(lock, [&] {return !job_signal_ || terminate_signal_; });
			}

			bool isReady()
			{
				std::unique_lock<std::mutex> lock(condition_mutex_);
				return !job_signal_;
			}

		private:
			void execute()
			{
				while(true)
				{
					std::unique_lock<std::mutex> lock(condition_mutex_);
					condition_var_.wait(lock, [&] {return job_signal_ || terminate_signal_; });

					if (job_signal_)
					{
						func_();
						job_signal_ = false;
					}

					condition_var_.notify_all();

					if (terminate_signal_)
						break;
				}
			}

		private:
			std::thread thread_instance_;

			std::mutex condition_mutex_;
			std::condition_variable condition_var_;
			std::atomic_bool terminate_signal_ = false;

			std::atomic_bool job_signal_ = false;
			std::function<void(void)> func_;
		};

		class JobSystemWorker;

		class JobSystem
		{
			friend JobSystemWorker;

		public:
			JobSystem();
			~JobSystem();

			void init(int num_max_thread);

			void add(std::function<void(void)> func);

			void waitAll();

		private:
			std::mutex condition_mutex_;
			std::condition_variable condition_var_;

			std::initializer_list<std::function<void(void)>> job_queue_{};

			std::vector<JobSystemWorker*> worker_thread_{};
		};
	}
}

#endif

