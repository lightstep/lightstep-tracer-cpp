#include "recorder/stream_recorder/utility.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

#include "common/platform/string.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// SeparateEndpoints
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<const char*>, std::vector<std::pair<int, uint16_t>>>
SeparateEndpoints(
    const std::vector<std::pair<std::string, uint16_t>>& endpoints) {
  std::vector<const char*> hosts;
  hosts.reserve(endpoints.size());
  for (auto& endpoint : endpoints) {
    hosts.emplace_back(endpoint.first.c_str());
  }
  auto less_than = [](const char* lhs, const char* rhs) {
    return StrCaseCmp(lhs, rhs) < 0;
  };
  auto equal_to = [](const char* lhs, const char* rhs) {
    return StrCaseCmp(lhs, rhs) == 0;
  };
  std::sort(hosts.begin(), hosts.end(), less_than);
  hosts.erase(std::unique(hosts.begin(), hosts.end(), equal_to), hosts.end());

  std::vector<std::pair<int, uint16_t>> indexed_endpoints;
  indexed_endpoints.reserve(endpoints.size());
  for (auto& endpoint : endpoints) {
    auto iter = std::lower_bound(hosts.begin(), hosts.end(),
                                 endpoint.first.c_str(), less_than);
    assert(iter != hosts.end());
    indexed_endpoints.emplace_back(
        static_cast<int>(std::distance(hosts.begin(), iter)), endpoint.second);
  }
  return {std::move(hosts), std::move(indexed_endpoints)};
}
}  // namespace lightstep
