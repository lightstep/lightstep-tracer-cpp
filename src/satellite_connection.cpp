#include "satellite_connection.h"

#include "address_info.h"
#include "utility.h"

#include <arpa/inet.h>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SatelliteConnection::SatelliteConnection(Logger& logger, const char* host,
                                         uint16_t port)
    : logger_{logger}, host_{host}, port_{port} {
  connect();
}

//------------------------------------------------------------------------------
// reconnect
//------------------------------------------------------------------------------
bool SatelliteConnection::reconnect() noexcept try {
  return true;
} catch (const std::exception& e) {
  logger_.Error("Failed to connect to satellite: ", e.what());
  return false;
}

//------------------------------------------------------------------------------
// reconnect
//------------------------------------------------------------------------------
bool SatelliteConnection::connect() noexcept try {
  auto address_info_list = GetAddressInfo(host_.c_str(), port_);
  is_connected_ =
      !address_info_list.forEachAddressInfo([&](const addrinfo& address_info) {
        logger_.Debug("Attempting to connect to satellite ",
                      AddressToString(*address_info.ai_addr));
        auto rcode = ::connect(socket_.file_descriptor(), address_info.ai_addr,
                               address_info.ai_addrlen);
        if (rcode < 0) {
          logger_.Debug("Failed to connect to satellite ",
                        AddressToString(*address_info.ai_addr));
          return true;
        }
        return false;
      });
  return is_connected_;
} catch (const std::exception& e) {
  logger_.Error("Failed to connect to satellite: ", e.what());
  return false;
}
}  // namespace lightstep
