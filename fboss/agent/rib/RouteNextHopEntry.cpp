/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "RouteNextHopEntry.h"

#include "fboss/agent/FbossError.h"

namespace {
constexpr auto kNexthops = "nexthops";
constexpr auto kAction = "action";
constexpr auto kAdminDistance = "adminDistance";
} // namespace

namespace facebook {
namespace fboss {

namespace util {

RouteNextHopSet toRouteNextHopSet(std::vector<NextHopThrift> const& nhs) {
  RouteNextHopSet rnhs;
  rnhs.reserve(nhs.size());
  for (auto const& nh : nhs) {
    rnhs.emplace(fromThrift(nh));
  }
  return rnhs;
}

std::vector<NextHopThrift> fromRouteNextHopSet(RouteNextHopSet const& nhs) {
  std::vector<NextHopThrift> nhts;
  nhts.reserve(nhs.size());
  for (const auto& nh : nhs) {
    nhts.emplace_back(nh.toThrift());
  }
  return nhts;
}

} // namespace util

RouteNextHopEntry::RouteNextHopEntry(NextHopSet nhopSet, AdminDistance distance)
    : adminDistance_(distance),
      action_(Action::NEXTHOPS),
      nhopSet_(std::move(nhopSet)) {
  if (nhopSet_.size() == 0) {
    throw FbossError("Empty nexthop set is passed to the RouteNextHopEntry");
  }
}

NextHopWeight RouteNextHopEntry::getTotalWeight() const {
  return totalWeight(getNextHopSet());
}

std::string RouteNextHopEntry::str() const {
  std::string result;
  switch (action_) {
    case Action::DROP:
      result = "DROP";
      break;
    case Action::TO_CPU:
      result = "To_CPU";
      break;
    case Action::NEXTHOPS:
      toAppend(getNextHopSet(), &result);
      break;
    default:
      CHECK(0);
      break;
  }
  result +=
      folly::to<std::string>(";admin=", static_cast<int32_t>(adminDistance_));
  return result;
}

bool operator==(const RouteNextHopEntry& a, const RouteNextHopEntry& b) {
  return (
      a.getAction() == b.getAction() and
      a.getNextHopSet() == b.getNextHopSet() and
      a.getAdminDistance() == b.getAdminDistance());
}

bool operator<(const RouteNextHopEntry& a, const RouteNextHopEntry& b) {
  if (a.getAdminDistance() != b.getAdminDistance()) {
    return a.getAdminDistance() < b.getAdminDistance();
  }
  return (
      (a.getAction() == b.getAction()) ? (a.getNextHopSet() < b.getNextHopSet())
                                       : a.getAction() < b.getAction());
}

// Methods for RouteNextHopEntry
void toAppend(const RouteNextHopEntry& entry, std::string* result) {
  result->append(entry.str());
}
std::ostream& operator<<(std::ostream& os, const RouteNextHopEntry& entry) {
  return os << entry.str();
}

// Methods for RouteNextHopEntry::NextHopSet
void toAppend(const RouteNextHopEntry::NextHopSet& nhops, std::string* result) {
  for (const auto& nhop : nhops) {
    result->append(folly::to<std::string>(nhop.str(), " "));
  }
}

std::ostream& operator<<(
    std::ostream& os,
    const RouteNextHopEntry::NextHopSet& nhops) {
  for (const auto& nhop : nhops) {
    os << nhop.str() << " ";
  }
  return os;
}

NextHopWeight totalWeight(const RouteNextHopEntry::NextHopSet& nhops) {
  uint32_t result = 0;
  for (const auto& nh : nhops) {
    result += nh.weight();
  }
  return result;
}

folly::dynamic RouteNextHopEntry::toFollyDynamic() const {
  folly::dynamic entry = folly::dynamic::object;
  entry[kAction] = forwardActionStr(action_);
  folly::dynamic nhops = folly::dynamic::array;
  for (const auto& nhop : nhopSet_) {
    nhops.push_back(nhop.toFollyDynamic());
  }
  entry[kNexthops] = std::move(nhops);
  entry[kAdminDistance] = static_cast<int32_t>(adminDistance_);
  return entry;
}

RouteNextHopEntry RouteNextHopEntry::fromFollyDynamic(
    const folly::dynamic& entryJson) {
  Action action = str2ForwardAction(entryJson[kAction].asString());
  auto it = entryJson.find(kAdminDistance);
  AdminDistance adminDistance = (it == entryJson.items().end())
      ? AdminDistance::MAX_ADMIN_DISTANCE
      : AdminDistance(entryJson[kAdminDistance].asInt());
  RouteNextHopEntry entry(Action::DROP, adminDistance);
  entry.action_ = action;
  for (const auto& nhop : entryJson[kNexthops]) {
    entry.nhopSet_.insert(util::nextHopFromFollyDynamic(nhop));
  }
  return entry;
}

} // namespace fboss
} // namespace facebook
