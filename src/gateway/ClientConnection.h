/*
 * Copyright (c) 2016 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ClientStates.h"
#include "SessionManager.h"
#include "FilterTypes.h"
#include <game_protocol/Packet.h>
#include <game_protocol/PacketHeaders.h>
#include <game_protocol/Handler.h>
#include <logger/Logging.h>
#include <spark/buffers/ChainedBuffer.h>
#include <shared/memory/ASIOAllocator.h>
#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <cstdint>

namespace ember {

class ClientConnection final : public std::enable_shared_from_this<ClientConnection> {
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_service& service_;

	enum class ReadState { HEADER, BODY, DONE } read_state_;

	ClientStates state_;
	protocol::Handler handler_;
	spark::ChainedBuffer<1024> inbound_buffer_;
	spark::ChainedBuffer<4096> outbound_buffer_;
	SessionManager& sessions_;
	ASIOAllocator allocator_; // temp - should be passed in
	log::Logger* logger_;
	bool stopped_;
	bool authenticated_;

	protocol::ClientHeader packet_header_;

	void read();
	void write();
	void stop();

	bool handle_packet(spark::Buffer& buffer);
	void handle_authentication(spark::Buffer& buffer);
	void send_auth_challenge();
	void parse_header(spark::Buffer& buffer);
	void completion_check(spark::Buffer& buffer);
	void dispatch_packet(spark::Buffer& buffer);

public:
	ClientConnection(SessionManager& sessions, boost::asio::io_service& service, log::Logger* logger)
	                 : state_(ClientStates::INITIAL_CONNECTION), sessions_(sessions), socket_(service),
	                   logger_(logger), read_state_(ReadState::HEADER), stopped_(false), service_(service) { }

	void start();
	void close_session();
	void send(protocol::ServerOpcodes opcode, std::shared_ptr<protocol::Packet> packet);
	boost::asio::ip::tcp::socket& socket();
	
	std::string remote_address();
	std::uint16_t remote_port();

	friend class SessionManager;
};

} // ember