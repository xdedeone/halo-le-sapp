#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <boost\asio\handler_invoke_hook.hpp>
#include <boost\asio\handler_type.hpp>
#include <boost\asio\spawn.hpp>
#include <boost\noncopyable.hpp>

#include "Semaphore.h"

struct async_work
{
	std::function<void()> f;
	boost::asio::handler_type<boost::asio::yield_context&, void()>::type handler;

	async_work(const std::function<void()> &f, boost::asio::handler_type<boost::asio::yield_context&, void()>::type handler) : f(f), handler(handler) {}
};

class thread_pool;

class thread_pool_thread : private boost::noncopyable
{
	friend class thread_pool;

	thread_pool* my_pool;
	std::unique_ptr<std::thread> my_thread;

public:
	thread_pool_thread(thread_pool* pool) : my_pool(pool)
	{
	}

	~thread_pool_thread()
	{
		my_thread->join();
	}

	void start()
	{
		my_thread = std::make_unique<std::thread>(&thread_pool_thread::thread_pool_func, this);
	}

protected:
	void thread_pool_func();
};


class thread_pool : private boost::noncopyable
{
	friend class thread_pool_thread;
	
	bool shutdown_ = false;

	size_t thread_count;

	std::vector<std::unique_ptr<thread_pool_thread>> threads;

	std::queue<std::unique_ptr<async_work>> works;
	std::mutex work_lock;

	ember::Semaphore<std::mutex> wSemaphore;

public:
	thread_pool(size_t thread_count) : thread_count(thread_count)
	{
	}

	~thread_pool()
	{		
	}

	void start()
	{
		for (size_t i = 0; i < thread_count; i++)
		{
			threads.emplace_back(std::make_unique<thread_pool_thread>(this));
			threads.back()->start();
		}
	}

	void post(std::unique_ptr<async_work> w)
	{
		std::unique_lock<std::mutex> guard(work_lock);
		works.push(std::move(w));
		guard.unlock();
		wSemaphore.signal();
	}

	void shutdown()
	{
		shutdown_ = true;
		wSemaphore.signal(thread_count);
		threads.clear();
	}
};