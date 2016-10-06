/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <spark/Link.h>
#include <spark/EventHandler.h>
#include <spark/temp/MessageRoot_generated.h>
#include <logger/Logging.h>
#include <boost/asio.hpp>
#include <chrono>
#include <forward_list>
#include <mutex>
#include <set>

namespace ember::spark {

class Service;

class HeartbeatService : public EventHandler {
	const std::chrono::seconds PING_FREQUENCY { 20 };
	const std::chrono::milliseconds LATENCY_WARN_THRESHOLD { 1000 };

	const Service* service_;
	std::forward_list<Link> peers_;
	std::mutex lock_;
	boost::asio::basic_waitable_timer<std::chrono::steady_clock> timer_;

	log::Logger* logger_;
	log::Filter filter_;

	void set_timer();
	void send_ping(const Link& link, std::uint64_t time);
	void send_pong(const Link& link, std::uint64_t time);
	void trigger_pings(const boost::system::error_code& ec);
	void handle_ping(const Link& link, const messaging::MessageRoot* message);
	void handle_pong(const Link& link, const messaging::MessageRoot* message);

public:
	HeartbeatService(boost::asio::io_service& io_service, const Service* service,
	                 log::Logger* logger, log::Filter filter);

	void handle_message(const Link& link, const messaging::MessageRoot* message);
	void handle_link_event(const Link& link, LinkState state);
	void shutdown();
};

}}