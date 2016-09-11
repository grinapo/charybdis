/*
 * Copyright (C) 2016 Charybdis Development Team
 * Copyright (C) 2016 Jason Volk <jason@zemos.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once
#define HAVE_IRCD_UTIL_TIMER_H

#ifdef __cplusplus

namespace ircd {
inline namespace util {

struct timer
{
	using clock = std::chrono::steady_clock;

	clock::time_point start;
	clock::time_point stop;

	template<class duration = std::chrono::seconds> duration get() const;
	template<class duration = std::chrono::seconds> duration mark();

	timer(const std::function<void ()> &);
	timer();
};

inline
timer::timer()
:start{clock::now()}
,stop{clock::time_point::min()}
{
}

inline
timer::timer(const std::function<void ()> &func)
:timer{}
{
	func();
	mark();
}

template<class duration>
duration
timer::mark()
{
	stop = clock::now();
	return std::chrono::duration_cast<duration>(stop - start);
}

template<class duration>
duration
timer::get()
const
{
	static const auto min(clock::time_point::min());
	const clock::time_point stop(this->stop != min? this->stop : clock::now());
	return std::chrono::duration_cast<duration>(stop - start);
}

}        // namespace util
}        // namespace ircd
#endif   // __cplusplus
