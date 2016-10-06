/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <spark/Link.h>
#include <spark/ServicesMap.h>
#include <spark/temp/MessageRoot_generated.h>
#include <logger/Logging.h>
#include <set>
#include <vector>
#include <cstdint>

namespace ember::spark {

class NetworkSession;
class EventDispatcher;
class LinkMap;

class MessageHandler {
	enum class State {
		HANDSHAKING, NEGOTIATING, FORWARDING
	} state_ = State::HANDSHAKING;

	Link peer_;
	const Link& self_;
	const EventDispatcher& dispatcher_;
	ServicesMap& services_;
	log::Logger* logger_;
	log::Filter filter_;
	std::set<std::int32_t> matches_;
	bool initiator_;

	void dispatch_message(const messaging::MessageRoot* message);
	bool negotiate_protocols(NetworkSession& net, const messaging::MessageRoot* message);
	bool establish_link(NetworkSession& net, const messaging::MessageRoot* message);
	void send_banner(NetworkSession& net);
	void send_negotiation(NetworkSession& net);

public:
	MessageHandler(const EventDispatcher& dispatcher, ServicesMap& services, const Link& link,
	               bool initiator, log::Logger* logger, log::Filter filter);
	~MessageHandler();

	bool handle_message(NetworkSession& net, const std::vector<std::uint8_t>& buffer);
	void start(NetworkSession& net);
};

} // spark, ember
