// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2019 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

// app.cc
namespace ircd::m::app
{
	void init();
	void fini();

	extern const m::room::id::buf app_room_id;
	extern std::map<std::string, m::room::id::buf> apps;
}
