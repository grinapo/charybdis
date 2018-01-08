/*
 * Copyright (C) 2017 Charybdis Development Team
 * Copyright (C) 2017 Jason Volk <jason@zemos.net>
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
 */

#pragma once
#define HAVE_IRCD_NET_H

/// Network IO subsystem.
///
/// Some parts of this system are not automatically included here when they
/// involve types which cannot be forward declared without boost headers.
/// This should not concern most developers as we have wrapped (or you should
/// wrap!) anything we need to expose to the rest of the project, or low-level
/// access may be had by including the asio.h header.
///
namespace ircd::net
{
	struct init;
	struct socket;

	IRCD_EXCEPTION(ircd::error, error)
	IRCD_EXCEPTION(error, invalid_argument)
	IRCD_EXCEPTION(error, nxdomain)
	IRCD_EXCEPTION(error, broken_pipe)
	IRCD_EXCEPTION(error, disconnected)
	IRCD_EXCEPTION(error, inauthentic)

	// SNOMASK 'N' "net"
	extern struct log::log log;
}

#include "hostport.h"
#include "ipport.h"
#include "remote.h"
#include "resolve.h"
#include "listener.h"
#include "sock_opts.h"
#include "open.h"
#include "close.h"
#include "read.h"
#include "write.h"

namespace ircd::net
{
	bool connected(const socket &) noexcept;
	size_t readable(const socket &);
	size_t available(const socket &) noexcept;
	ipport local_ipport(const socket &) noexcept;
	ipport remote_ipport(const socket &) noexcept;
	const_raw_buffer peer_cert_der(const mutable_raw_buffer &, const socket &);
}

// Exports to ircd::
namespace ircd
{
	using net::socket;
}

struct ircd::net::init
{
	init();
	~init() noexcept;
};
