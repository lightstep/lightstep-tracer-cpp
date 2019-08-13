#include "network/vector_write.h"

#include <vector>

#include "3rd_party/catch2/catch.hpp"
#include "common/fragment_array_input_stream.h"
#include "network/socket.h"
#include "test/echo_server/echo_server_handle.h"
#include "test/ports.h"
#include "test/utility.h"

#include <sys/uio.h>
using namespace lightstep;

const auto TcpPort = static_cast<uint16_t>(PortAssignments::VectorWriteTestTcp);

TEST_CASE("VectorWrite") {
  EchoServerHandle echo_server{
      static_cast<uint16_t>(PortAssignments::VectorWriteTestHttp), TcpPort};

  FragmentArrayInputStream fragments1{MakeFragment("abc"), MakeFragment("123")};
  FragmentArrayInputStream fragments2{MakeFragment("xyz"), MakeFragment("")};
  Socket socket;
  socket.SetReuseAddress();
  IpAddress server_address{"127.0.0.1", TcpPort};
  REQUIRE(socket.Connect(server_address.addr(),
                         sizeof(server_address.ipv4_address())) == 0);

  SECTION("Calling write with no fragments returns true.") {
    REQUIRE(Write(socket.file_descriptor(), {}));
  }

  SECTION("Write can be used to send fragments over a socket.") {
    auto result = Write(socket.file_descriptor(), {&fragments1, &fragments2});
    socket = Socket{};
    REQUIRE(result);
    REQUIRE(
        IsEventuallyTrue([&] { return echo_server.data() == "abc123xyz"; }));
  }

  SECTION(
      "On a non-blocking socket, the vectorized write returns immediately with "
      "false if the socket would block.") {
    socket.SetNonblocking();
    std::vector<char> big_array(100000, ' ');
    int i = 0;
    int max_iterations = 1000;
    for (; i < max_iterations; ++i) {
      FragmentArrayInputStream fragments{{static_cast<void*>(big_array.data()),
                                          static_cast<int>(big_array.size())},
                                         {static_cast<void*>(big_array.data()),
                                          static_cast<int>(big_array.size())},
                                         {static_cast<void*>(big_array.data()),
                                          static_cast<int>(big_array.size())},
                                         {static_cast<void*>(big_array.data()),
                                          static_cast<int>(big_array.size())}};
      auto result = Write(socket.file_descriptor(), {&fragments});
      if (!result) {
        break;
      }
    }
    REQUIRE(i < max_iterations);
  }

  SECTION("We can write more fragments than IOV_MAX.") {
    for (int num_fragments :
         {static_cast<int>(IOV_MAX) - 1, static_cast<int>(IOV_MAX),
          static_cast<int>(IOV_MAX) + 1, 2 * static_cast<int>(IOV_MAX),
          3 * static_cast<int>(IOV_MAX) / 2}) {
      SECTION("num_fragments = " + std::to_string(num_fragments)) {
        FragmentArrayInputStream fragments;
        std::array<char, 1> data = {'X'};
        for (int i = 0; i < num_fragments; ++i) {
          fragments.Add(Fragment{static_cast<void*>(data.data()), 1});
        }
        auto result = Write(socket.file_descriptor(), {&fragments});
        socket = Socket{};
        REQUIRE(result);
        REQUIRE(IsEventuallyTrue([&] {
          return echo_server.data() == std::string(num_fragments, 'X');
        }));
      }
    }
  }

  SECTION("Write throws an exception when there's an error.") {
    socket.SetNonblocking();
    auto file_descriptor = socket.file_descriptor();
    socket = Socket{};
    REQUIRE_THROWS(Write(file_descriptor, {&fragments1, &fragments2}));
  }
}
