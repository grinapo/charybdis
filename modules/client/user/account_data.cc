// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

#include "user.h"

using namespace ircd;

resource::response
put__account_data(client &client,
                  const resource::request &request,
                  const m::user::id &user_id)
{
	return resource::response
	{
		client, http::OK
	};
}
