
// Copyright (c) 2013,2014, niXman (i dotty nixman doggy gmail dotty com)
// All rights reserved.
//
// This file is part of PROCESSOR(https://github.com/niXman/processor) project.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or
//   other materials provided with the distribution.
//
//   Neither the name of the {organization} nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _processor_hpp
#define _processor_hpp

#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <list>

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/intrusive/list.hpp>

/***************************************************************************/

template<typename F, typename H>
struct processor: private boost::noncopyable {
	processor(const std::size_t nthreads)
		:ios(nthreads)
		,work(new boost::asio::io_service::work(ios))
	{
		for ( auto idx = 0u; idx < nthreads; ++idx ) {
			threads.emplace_back([this](){ios.run();});
		}
	}

	virtual ~processor() {
		work.reset();
		for ( auto& it: threads ) {
			it.join();
		}
		ios.run();
	}

	void add(const std::shared_ptr<char> &buf, const std::size_t size, F func, H handler) {
		task *t = new task(buf, size, handler);

		{
			std::lock_guard<std::mutex> lock(mutex);
			tasks.push_back(*t);
		}

		auto proc = [this, func, t]() {
			auto res = func(t->buf, t->size);
			t->buf  = res.first;
			t->size = res.second;

			{
				std::lock_guard<std::mutex> lock(mutex);
				t->processed = true;

				while ( !tasks.empty() ) {
					task *st = &tasks.front();
					if ( st->processed ) {
						tasks.pop_front();

						st->handler(st->buf, st->size);
						delete st;
					} else {
						break;
					}
				}
			}
		};

		ios.post(std::move(proc));
	}

private:
	struct task: boost::intrusive::list_base_hook<> {
		task(const std::shared_ptr<char> &buf, const std::size_t size, H handler)
			:buf(buf)
			,size(size)
			,processed(false)
			,handler(std::move(handler))
		{}

		std::shared_ptr<char> buf;
		std::size_t size;
		bool processed;
		H handler;
	};

	std::mutex mutex;
	boost::intrusive::list<task> tasks;
private:
	boost::asio::io_service ios;
	std::unique_ptr<boost::asio::io_service::work> work;
	std::list<std::thread> threads;
};

/***************************************************************************/

#endif // _processor_hpp
