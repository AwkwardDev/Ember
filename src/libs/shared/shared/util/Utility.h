/*
 * Copyright (c) 2016 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <cstddef>

namespace ember::util {

std::size_t max_consecutive_check(const std::string& name);
void set_window_title(const std::string& title);

} // util, ember
