/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <spark/temp/MessageRoot_generated.h>
#include <spark/temp/Multicast_generated.h>
#include <optional>
#include <boost/uuid/uuid.hpp>

namespace ember::spark {

struct Link;
enum class LinkState;
struct Endpoint;
class ServiceDiscovery;

//namespace messaging { struct MessageRoot; }

typedef std::function<void(const spark::Link&, const boost::uuids::uuid&,
	std::optional<const messaging::MessageRoot*>)> TrackingHandler;
typedef std::function<void(const Endpoint*)> ResolveCallback;
typedef std::function<void(const messaging::multicast::LocateAnswer*)> LocateCallback;

} // spark, ember
