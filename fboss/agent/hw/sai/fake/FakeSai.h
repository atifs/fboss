/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include "FakeSaiPort.h"
#include "FakeSaiSwitch.h"

#include <memory>

extern "C" {
  #include <sai.h>
}

namespace facebook {
namespace fboss {

struct FakeSai {
  static std::shared_ptr<FakeSai> getInstance();
  FakePortManager pm;
  FakeSwitchManager swm;
  bool initialized = false;
};

} // namespace fboss
} // namespace facebook

sai_status_t sai_api_initialize(
    uint64_t flags,
    const sai_service_method_table_t* services);

sai_status_t sai_api_query(sai_api_t sai_api_id, void** api_method_table);
