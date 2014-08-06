
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

#include <iostream>

#include "processor.hpp"

/***************************************************************************/

int main() {
	try {
		auto func = [](const std::shared_ptr<char> &buf, const std::size_t size){
			return std::make_pair(buf, size);
		};
		auto hand = [](const std::shared_ptr<char> &buf, const std::size_t size){
			std::cout << "ptr=" << (void*)buf.get() << ", size=" << size << ", thid=" << std::this_thread::get_id() << std::endl;
		};

		processor<decltype(func), decltype(hand)> p(8);

		for ( std::size_t idx = 0; idx < 1024; ++idx ) {
			const std::size_t size = rand()%(1024*1024);
			std::shared_ptr<char> buf(new char[size]);
			p.add(buf, size, func, hand);
		}
	} catch (const std::exception &ex) {
		std::cerr << "[std::exception]: " << ex.what() << std::endl;
	} catch (...) {
		std::cerr << "[unknown exception] " << std::endl;
	}
}

/***************************************************************************/
