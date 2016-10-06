/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <spark/MessageHandler.h>
#include <spark/SessionManager.h>
#include <spark/buffers/ChainedBuffer.h>
#include <logger/Logging.h>
#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>
#include <flatbuffers/flatbuffers.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace ember::spark {

class NetworkSession : public std::enable_shared_from_this<NetworkSession> {
	const std::size_t MAX_MESSAGE_LENGTH = 1024 * 1024;  // 1MB
	const std::size_t DEFAULT_BUFFER_LENGTH = 1024 * 16; // 16KB
	enum class ReadState { HEADER, BODY };

	boost::asio::ip::tcp::socket socket_;
	boost::asio::strand strand_;

	ReadState state_;
	std::uint32_t body_read_size;
	std::vector<std::uint8_t> in_buff_;
	SessionManager& sessions_;
	MessageHandler handler_;
	const std::string remote_;
	log::Logger* logger_; 
	log::Filter filter_;
	bool stopped_;

	bool process_header() {
		std::memcpy(&body_read_size, in_buff_.data(), sizeof(body_read_size));
		boost::endian::little_to_native_inplace(body_read_size);
	
		if(body_read_size > MAX_MESSAGE_LENGTH) {
			LOG_WARN_FILTER(logger_, filter_)
				<< "[spark] Peer at " << remote_host()
				<< " attempted to send a message of "
				<< body_read_size << " bytes" << LOG_ASYNC;

			return false;
		}

		if(body_read_size > in_buff_.size()) {
			LOG_WARN_FILTER(logger_, filter_)
				<< "[spark] Peer at " << remote_host()
				<< " attempted to send a message of "
				<< body_read_size << " bytes" << LOG_ASYNC;

			in_buff_.resize(body_read_size);
		}

		return true;
	}

	void handle_read(boost::system::error_code ec) {
		if(ec) {
			if(ec != boost::asio::error::operation_aborted) {
				close_session();
			}

			return;
		}

		if(state_ == ReadState::HEADER) {
			if(process_header()) {
				state_ = ReadState::BODY;
				read();
				return;
			}
		} else if(handler_.handle_message(*this, in_buff_)) {
			state_ = ReadState::HEADER;
			read();
			return;
		}

		close_session();
	}

	void read() {
		auto self(shared_from_this());
		std::size_t read_size = state_ == ReadState::HEADER? sizeof(body_read_size) : body_read_size;

		boost::asio::async_read(socket_, boost::asio::buffer(in_buff_, read_size), strand_.wrap(
			[this, self](boost::system::error_code ec, std::size_t size) {
				if(!stopped_) {
					handle_read(ec);
				}
			}
		));
	}


	void stop() {
		LOG_DEBUG_FILTER(logger_, filter_)
			<< "[spark] Closing connection to " << remote_host() << LOG_ASYNC;

		stopped_ = true;
		boost::system::error_code ec; // we don't care about any errors
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		socket_.close(ec);
	}

public:
	NetworkSession(SessionManager& sessions, boost::asio::ip::tcp::socket socket, MessageHandler handler,
	               log::Logger* logger, log::Filter filter)
	               : sessions_(sessions), socket_(std::move(socket)), body_read_size(0),
	                 handler_(handler), logger_(logger), filter_(filter), stopped_(false),
	                 state_(ReadState::HEADER), in_buff_(DEFAULT_BUFFER_LENGTH),
	                 strand_(socket_.get_io_service()),
	                 remote_(socket_.remote_endpoint().address().to_string()
	                         + ":" + std::to_string(socket_.remote_endpoint().port())) { }

	void start() {
		handler_.start(*this);
		read();
	}

	void close_session() {
		sessions_.stop(shared_from_this());
	}

	std::string remote_host() {
		return remote_;
	}

	void write(std::shared_ptr<flatbuffers::FlatBufferBuilder> fbb) {
		if(!socket_.is_open()) {
			return;
		}

		auto size = static_cast<decltype(body_read_size)>(fbb->GetSize());

		if(size > MAX_MESSAGE_LENGTH) {
			LOG_DEBUG_FILTER(logger_, filter_)
				<< "[spark] Attempted to send a message larger than permitted size ("
				<< MAX_MESSAGE_LENGTH << " bytes)" << LOG_ASYNC;
			return;
		}

		boost::endian::native_to_little_inplace(size);

		// todo - remove this allocation - waiting for a better solution than can be achieved
		// with the current FlatBuffers custom allocator design
		auto size_ptr = std::make_shared<decltype(body_read_size)>(size);

		std::array<boost::asio::const_buffer, 2> buffers {
			boost::asio::const_buffer { size_ptr.get(), sizeof(body_read_size) },
			boost::asio::const_buffer { fbb->GetBufferPointer(), fbb->GetSize() }
		};

		auto self(shared_from_this());

		socket_.async_send(buffers, strand_.wrap(
			[this, self, fbb, size_ptr](boost::system::error_code ec, std::size_t /*size*/) {
				if(ec && ec != boost::asio::error::operation_aborted) {
					close_session();
				}
			}
		));
	}

	virtual ~NetworkSession() = default;

	friend class SessionManager;
};

} // spark, ember
