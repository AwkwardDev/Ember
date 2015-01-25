/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Logger.h"
#include "HelperMacros.h"
#include <memory>
#include <mutex>

namespace ember { namespace log {

namespace detail {

Logger* logger_;

} //detail

Logger* get_logger() {
	return detail::logger_;
}

void set_global_logger(Logger* logger) {
	detail::logger_ = logger;
}

}} //log, ember