#include "thread_pool.h"

void thread_pool_thread::thread_pool_func()
{
	while (true)
	{
		my_pool->wSemaphore.wait();
		if (!my_pool->shutdown_)
		{
			std::unique_lock<std::mutex> lock(my_pool->work_lock);
			std::unique_ptr<async_work> w = move(my_pool->works.front());
			my_pool->works.pop();
			lock.unlock();
			w->f();
			using boost::asio::asio_handler_invoke;
			asio_handler_invoke(w->handler, &w->handler);
		}
		else break;
	}
}