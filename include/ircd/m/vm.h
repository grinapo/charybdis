// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

#pragma once
#define HAVE_IRCD_M_VM_H

/// Matrix Virtual Machine
///
namespace ircd::m::vm
{
	struct error; // custom exception
	struct init;
	struct opts;
	struct copts;
	struct eval;
	enum fault :uint;
	using fault_t = std::underlying_type<fault>::type;

	extern const opts default_opts;
	extern const copts default_copts;
	extern log::log log;
	extern ctx::dock dock;
	extern bool ready;

	string_view reflect(const fault &);
	http::code http_code(const fault &);
	string_view loghead(const mutable_buffer &, const eval &);
	string_view loghead(const eval &);    // single tls buffer

	fault execute(eval &, const event &);
	fault inject(eval &, json::iov &, const json::iov &);
}

namespace ircd::m::vm::sequence
{
	extern ctx::dock dock;
	extern uint64_t retired;      // already written; always monotonic
	extern uint64_t committed;    // pending write; usually monotonic
	extern uint64_t uncommitted;  // evaluating; not monotonic
	static size_t pending;

	const uint64_t &get(const eval &);
	uint64_t get(id::event::buf &); // [GET]
	uint64_t max();
	uint64_t min();
};

struct ircd::m::vm::init
{
	init(), ~init() noexcept;
};

/// Event Evaluation Device
///
/// This object conducts the evaluation of an event or a tape of multiple
/// events. An event is evaluated in an attempt to execute it. Events which
/// fail during evaluation won't be executed; such is the case for events which
/// have already been executed, or events which are invalid or lead to invalid
/// transitions or actions of the machine etc.
///
struct ircd::m::vm::eval
:instance_list<eval>
{
	static uint64_t id_ctr;
	static uint executing;
	static uint injecting;

	const vm::opts *opts {&default_opts};
	const vm::copts *copts {nullptr};
	ctx::ctx *ctx {ctx::current};
	uint64_t id {++id_ctr};
	uint64_t sequence {0};
	uint64_t sequence_shared[2] {0}; // min, max
	std::shared_ptr<db::txn> txn;

	vector_view<m::event> pdus;
	const json::iov *issue {nullptr};
	const event *event_ {nullptr};
	string_view room_id;
	event::id::buf event_id;
	event::conforms report;
	string_view room_version;
	const hook::base::site *phase {nullptr};
	bool room_internal {false};

	static bool for_each_pdu(const std::function<bool (const event &)> &);
	static const event *find_pdu(const eval &, const event::id &);
	static const event *find_pdu(const event::id &);

	void mfetch_keys() const;

  public:
	operator const event::id::buf &() const;

	fault operator()(const event &);
	size_t operator()(const vector_view<m::event> &);
	fault operator()(json::iov &event, const json::iov &content); //inject

	eval(const vm::opts &);
	eval(const vm::copts &);
	eval(const event &, const vm::opts & = default_opts);
	eval(const vector_view<m::event> &, const vm::opts & = default_opts);
	eval(const json::array &event, const vm::opts & = default_opts);
	eval(json::iov &event, const json::iov &content, const vm::copts & = default_copts);
	eval() = default;
	eval(eval &&) = delete;
	eval(const eval &) = delete;
	eval &operator=(eval &&) = delete;
	eval &operator=(const eval &) = delete;
	~eval() noexcept;

	static bool for_each(const ctx::ctx *const &, const std::function<bool (eval &)> &);
	static bool for_each(const std::function<bool (eval &)> &);
	static size_t count(const ctx::ctx *const &);
	static size_t count(const event::id &);
	static eval *find(const event::id &);
	static eval &get(const event::id &);
	static bool sequnique(const uint64_t &seq);
	static eval *seqnext(const uint64_t &seq);
	static eval *seqmax();
	static eval *seqmin();
	static void seqsort();
};

/// Evaluation faults. These are reasons which evaluation has halted but may
/// continue after the user defaults the fault. They are basically types of
/// interrupts and traps, which are supposed to be recoverable. Only the
/// GENERAL protection fault (#gp) is an abort and is not supposed to be
/// recoverable. The fault codes have the form of bitflags so they can be
/// used in masks; outside of that case only one fault is dealt with at
/// a time so they can be switched as they appear in the enum.
///
enum ircd::m::vm::fault
:uint
{
	ACCEPT        = 0x00,  ///< No fault.
	EXISTS        = 0x01,  ///< Replaying existing event. (#ex)
	GENERAL       = 0x02,  ///< General protection fault. (#gp)
	INVALID       = 0x04,  ///< Non-conforming event format. (#ud)
	AUTH          = 0x08,  ///< Auth rules violation. (#av)
	STATE         = 0x10,  ///< Required state is missing (#st)
	EVENT         = 0x20,  ///< Eval requires addl events in the ef register (#ef)
};

/// Evaluation Options
struct ircd::m::vm::opts
{
	/// The remote server name which is conducting this eval.
	string_view node_id;

	/// The mxid of the user which is conducting this eval.
	string_view user_id;

	/// The txnid from the node conducting the eval.
	string_view txn_id;

	/// Call conform hooks (detailed behavior can be tweaked below)
	bool conform {true};

	/// Check various access controls before processing event further.
	bool access {true};

	/// Make fetches or false to bypass fetch stage.
	bool fetch {true};

	/// Call eval hooks or false to bypass this stage.
	bool eval {true};

	/// Perform auth or false to bypass this state.
	bool auth {true};

	/// Make writes to database
	bool write {true};

	/// Custom write_opts to use during write.
	dbs::write_opts wopts;

	/// Call post hooks or false to bypass post-write / pre-notify effects.
	bool post {true};

	/// Broadcast to clients/servers. When true, individual notify options
	/// that follow are considered. When false, no notifications occur.
	short notify {true};

	/// Broadcast to local clients (/sync stream).
	bool notify_clients {true};

	/// Broadcast to federation servers (/federation/send/).
	bool notify_servers {true};

	/// Apply effects of this event or false to bypass this stage.
	bool effects {true};

	/// False to allow a dirty conforms report (not recommended).
	bool conforming {true};

	/// Mask of conformity failures to allow without considering dirty.
	event::conforms non_conform;

	/// If the event was already checked before the eval, set this to true
	/// and include the report (see below).
	bool conformed {false};

	/// When conformed=true, this report will be included instead of generating
	/// one during the eval. This is useful if a conformity check was already
	/// done before eval.
	event::conforms report;

	/// Supply the room version; overrides/avoids any internal query.
	string_view room_version;

	/// Toggles whether event may be considered a "present event" and may
	/// update the optimized present state table of the room if it is proper.
	bool present {true};

	/// Toggles whether event may be added to the room head table which means
	/// it is considered unreferenced by any other event at this time. It is
	/// safe for this to always be true if events are evaluated in order. If
	/// `present` is false this should be set to false but they are not tied.
	bool room_head {true};

	/// Toggles whether the prev_events of this event are removed from the
	/// room head table, now that this event has referenced them. It is safe
	/// for this to always be true.
	bool room_head_resolve {true};

	/// Toggles whether the state btree is updated; this should be consistently
	/// true or false for all events in a room.
	bool history {true};

	/// Evaluate in EDU mode. Input must not have event_id and none will be
	/// generated for it.
	bool edu {false};

	/// Bypass check for event having already been evaluated so it can be
	/// replayed through the system (not recommended).
	bool replays {false};

	/// Bypass check for another evaluation of the same event_id already
	/// occurring. If this is false (not recommended) two duplicate events
	/// being evaluated may race through the core.
	bool unique {true};

	/// If the input event has a reference to already-strung json we can use
	/// that directly when writing to the DB. When this is false we will
	/// re-stringify the event internally either from a referenced source or
	/// the tuple if no source is referenced. This should only be set to true
	/// if the evaluator already performed this and the json source is good.
	bool json_source {false};

	/// Verify the origin signature; recommended.
	bool verify {true};

	/// Whether to gather all unknown keys from an input vector of events and
	/// perform a parallel/mass fetch before proceeding with the evals.
	bool mfetch_keys {true};

	/// Whether to automatically fetch the auth events when they do not exist.
	bool fetch_auth {true};

	/// Whether to automatically fetch the room state when there is no state
	/// or incomplete state for the room found on the this server.
	bool fetch_state {true};

	/// Dispatches a fetch operation when a prev_event does not exist locally.
	bool fetch_prev {true};

	/// Throws fault::EVENT if *all* of the prev_events do not exist locally.
	/// This is used to enforce that at least one path is traversable. This
	/// test is conducted after waiting if fetch_prev and fetch_prev_wait.
	bool fetch_prev_any {false};

	/// Throws fault::EVENT if *any* of the prev_events do not exist locally.
	/// This is used to enforce that all references have been acquired; other
	/// corollary conditions are similar to fetch_prev_any.
	bool fetch_prev_all {false};

	/// The number of iterations of the wait cycle which checks for missing
	/// prev_events will iterate before issuing remote fetches for them.
	/// The default is 0 which bypasses the functionality, and is recommended
	/// when the evaluator is confident missing prev_events won't arrive
	/// elsehow. Setting to -1 enables it with an auto/conf value.
	size_t fetch_prev_wait_count {0};

	/// Base time to wait for missing prev_events to arrive at the server by
	/// some other means before issuing remote fetches for them. The waiting
	/// occurs in a loop where prev_events satisfaction is checked at each
	/// iteration. This value is multiplied by the number of iterations for
	/// multiplicative backoff. The default of -1 is auto / conf.
	milliseconds fetch_prev_wait_time {-1};

	/// The limit on the number of events to backfill if any of the prev_events
	/// are missing. -1 is auto / conf.
	size_t fetch_prev_limit = -1;

	/// Evaluators can set this value to optimize the creation of the database
	/// transaction where the event will be stored. This value should be set
	/// to the amount of space the event consumes; the JSON-serialized size is
	/// a good value here. Default of -1 will automatically use serialized().
	size_t reserve_bytes = -1;

	/// This value is added to reserve_bytes to account for indexing overhead
	/// in the database transaction allocation. Most evaluators have little
	/// reason to ever adjust this.
	size_t reserve_index {1024};

	/// Mask of faults that are not thrown as exceptions out of eval(). If
	/// masked, the fault is returned from eval(). By default, the EXISTS
	/// fault is masked which means existing events won't kill eval loops.
	fault_t nothrows
	{
		EXISTS
	};

	/// Mask of faults that are logged to the error facility in vm::log.
	fault_t errorlog
	{
		~(EXISTS)
	};

	/// Mask of faults that are logged to the warning facility in vm::log
	fault_t warnlog
	{
		EXISTS
	};

	/// Whether to log a debug message on successful eval.
	bool debuglog_accept {false};

	/// Whether to log an info message on successful eval.
	bool infolog_accept {false};

	opts() noexcept;
};

/// Extension structure to vm::opts which includes additional options for
/// commissioning events originating from this server which are then passed
/// through eval (this process is also known as issuing).
struct ircd::m::vm::copts
:opts
{
	/// A matrix-spec opaque token from a client identifying this eval.
	string_view client_txnid;

	/// This bitmask covers all of the top-level properties of m::event
	/// which will be generated internally during injection unless they
	/// already exist. Clearing any of these bits will prevent the internal
	/// generation of these properties (i.e. for EDU's).
	event::keys::selection prop_mask
	{
		event::keys::include
		{
			"auth_events",
			"depth",
			"event_id",
			"hashes",
			"origin",
			"origin_server_ts",
			"prev_events",
			"prev_state",
			"signatures",
		}
	};

	/// Call the issue hook or bypass
	bool issue {true};

	/// Whether to log a debug message before commit
	bool debuglog_precommit {false};

	/// Whether to log an info message after commit accepted
	bool infolog_postcommit {false};

	copts() noexcept;
};

struct ircd::m::vm::error
:m::error
{
	vm::fault code;

	template<class... args> error(const http::code &, const fault &, const string_view &fmt, args&&... a);
	template<class... args> error(const fault &, const string_view &fmt, args&&... a);
	template<class... args> error(const string_view &fmt, args&&... a);
};

template<class... args>
ircd::m::vm::error::error(const string_view &fmt,
                          args&&... a)
:error
{
	http::INTERNAL_SERVER_ERROR, fault::GENERAL, fmt, std::forward<args>(a)...
}
{}

template<class... args>
ircd::m::vm::error::error(const fault &code,
                          const string_view &fmt,
                          args&&... a)
:error
{
	http_code(code), code, fmt, std::forward<args>(a)...
}
{}

template<class... args>
ircd::m::vm::error::error(const http::code &httpcode,
                          const fault &code,
                          const string_view &fmt,
                          args&&... a)
:m::error
{
	child, httpcode, reflect(code), fmt, std::forward<args>(a)...
}
,code
{
	code
}
{}
