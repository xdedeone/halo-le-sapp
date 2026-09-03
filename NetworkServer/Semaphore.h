/*
* Copyright (c) 2014 Ember
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <atomic>
#include <condition_variable>
//#include <chrono>

#undef max

namespace ember {

	template<typename Lock>
	class Semaphore {
		std::atomic<unsigned int> count_;
		std::condition_variable condition_;
		unsigned int max_;
		Lock lock_;

	public:
		Semaphore(unsigned int initial_count = 0,
			unsigned int max_count = std::numeric_limits<unsigned int>::max())
			: count_(initial_count) {
			max_ = max_count;
		}

		void wait() {
			std::unique_lock<Lock> guard(lock_);

			while (!count_) {
				condition_.wait(guard);
			}

			--count_;
		}

		//bool wait_for(std::chrono::milliseconds duration) {
		//	std::unique_lock<Lock> guard(lock_);

		//	//wait_for with a predicate is broken in VS2013 & VS2015 Preview - workaround! todo
		//	bool hack = false;

		//	condition_.wait_for(guard, std::chrono::milliseconds(duration),
		//		[&]() {
		//		if (count_ != 0) {
		//			--count_;
		//			hack = true;
		//			return true;
		//		}
		//		return false;
		//	});

		//	return hack;
		//}

		//void decrement_no_wait(unsigned int dec) {
		//	unsigned int curr = count_;
		//	unsigned int new_count;

		//	do {
		//		new_count = curr - dec;
		//	} while (!count_.compare_exchange_weak(curr, new_count));
		//}

		void signal(unsigned int increment = 1) {
			count_ += increment;
			for (unsigned int i = 0; i < increment; i++)
				condition_.notify_one();
		}
	};

} //ember