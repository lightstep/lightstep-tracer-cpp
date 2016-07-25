#include <boost/network/uri/encode.hpp>
#include <boost/network/uri/decode.hpp>

#include "propagation.h"

std::string uriQueryEscape(const std::string& str) {
  return boost::network::uri::encoded(str);
}

std::string uriQueryUnescape(const std::string& str) {
  return boost::network::uri::decoded(str);
}

