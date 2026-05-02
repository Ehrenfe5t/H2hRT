#pragma once

#include<iostream>
#include<thread>
#include<future>
#include<vector>
#include<queue>
#include<functional>
#include<memory>

#include"LxQProjectDependencies.h"

namespace ThreadPoolStd {

    class ThreadPool {
    public:
        explicit ThreadPool() {

            // 获取硬件线程数
            const unsigned int thread_count = std::thread::hardware_concurrency();

            for (unsigned int i = 0; i < thread_count; ++i) {
                threads_.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock{mutex_};
                            condition_.wait(lock, [this] { return !queue_.empty() || stop_; });
                            if (stop_ && queue_.empty())
                                return;
                            task = std::move(queue_.front());
                            queue_.pop();
                        }
                        task();
                    }
                    });
            }
        }

        ~ThreadPool() {
            //{
            //    std::unique_lock<std::mutex> lock{mutex_};
            //    stop_ = true;
            //}
            //condition_.notify_all();
            //for (auto& thread : threads_)
            //    thread.join();
        }

        template<typename Func, typename... Args>
        auto submit(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func, Args...>> {
            using return_type = std::invoke_result_t<Func, Args...>;
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
            );
            auto future_result = task->get_future();
            {
                std::lock_guard<std::mutex> lock{mutex_};
                queue_.emplace([task]() { (*task)(); });
            }
            condition_.notify_one();
            return future_result;
        }

        int GetCurTaskNum() {
            return (int)queue_.size();
        }

        void join() {
            int old_size = GetCurTaskNum();
            int print_old_size = old_size;
            {
                std::ostringstream oss;
                oss << "线程池中还有的任务量:" << print_old_size;
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
            }
            int printf_gap = (int)(0.01 * (double)old_size);
            if (printf_gap < 60) {
                printf_gap = 60;
            }
            while (old_size > 0) {
                if (old_size > printf_gap) {
                    if (print_old_size - old_size > printf_gap) {
                        print_old_size = old_size;
                        {
                            std::ostringstream oss;
                            oss << "线程池中还有的任务量:" << print_old_size;
                            ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
                else {
                    if (print_old_size - old_size > 3) {
                        print_old_size = old_size;
                        {
                            std::ostringstream oss;
                            oss << "线程池中还有的任务量:" << print_old_size;
                            ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                old_size = GetCurTaskNum();
            }

            {
                std::unique_lock<std::mutex> lock{mutex_};
                stop_ = true;
            }
            condition_.notify_all();
            for (auto& thread : threads_)
                thread.join();
        }


    private:
        std::vector<std::jthread> threads_;
        std::queue<std::function<void()>> queue_;
        std::mutex mutex_;
        std::condition_variable condition_;
        bool stop_ = false;
    };


	
}