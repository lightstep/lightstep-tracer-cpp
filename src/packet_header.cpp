#include "packet_header.h"

#include <lightstep/config.h>
#include "utility.h"

#include <algorithm>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
PacketHeader::PacketHeader(const char* data) noexcept {
  std::copy_n(data, sizeof(version_), reinterpret_cast<char*>(&version_));
  data += sizeof(version_);

  std::copy_n(data, sizeof(type_), reinterpret_cast<char*>(&type_));
  data += sizeof(type_);

  std::copy_n(data, sizeof(body_size_), reinterpret_cast<char*>(&body_size_));
#ifdef LIGHTSTEP_BIG_ENDIAN
  ReverseEndianness(body_size_);
#endif
}

PacketHeader::PacketHeader(PacketType type, uint32_t body_size) noexcept
    : type_{type}, body_size_{body_size} {}

PacketHeader::PacketHeader(uint8_t version, PacketType type, uint32_t body_size)
    : version_{version}, type_{type}, body_size_{body_size} {}

//------------------------------------------------------------------------------
// serialize
//------------------------------------------------------------------------------
void PacketHeader::serialize(char* data) const noexcept {
  std::copy_n(reinterpret_cast<const char*>(&version_), sizeof(version_), data);
  data += sizeof(version_);

  std::copy_n(reinterpret_cast<const char*>(&type_), sizeof(type_), data);
  data += sizeof(type_);

  auto body_size = body_size_;
#ifdef LIGHTSTEP_BIG_ENDIAN
  ReverseEndianness(body_size);
#endif
  std::copy_n(reinterpret_cast<const char*>(&body_size), sizeof(body_size),
              data);
}
}  // namespace lightstep
