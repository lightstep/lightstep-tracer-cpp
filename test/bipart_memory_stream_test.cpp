#include "../src/bipart_memory_stream.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;

TEST_CASE("BipartMemoryOutputStream") {
  SECTION("Next returns false when there's no data attached.") {
    BipartMemoryOutputStream buffer{nullptr, 0, nullptr, 0};
    void* data;
    int size;
    CHECK(!buffer.Next(&data, &size));
  }

  SECTION("Next returns the first memory buffer provided") {
    char data1[10];
    BipartMemoryOutputStream buffer{data1, 10, nullptr, 0};
    void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<char*>(data) == data1);
    CHECK(size == 10);
    CHECK(buffer.ByteCount() == 10);
    CHECK(!buffer.Next(&data, &size));
  }

  SECTION("Next returns the first, then second memory buffer if provided") {
    char data1[10];
    char data2[5];
    BipartMemoryOutputStream buffer{data1, 10, data2, 5};
    void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<char*>(data) == data1);
    CHECK(size == 10);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<char*>(data) == data2);
    CHECK(size == 5);
    CHECK(buffer.ByteCount() == 15);
    CHECK(!buffer.Next(&data, &size));
  }

  SECTION("BackUp to the first buffer returns thre correct partial buffer") {
    char data1[10];
    char data2[5];
    BipartMemoryOutputStream buffer{data1, 10, data2, 5};
    void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    buffer.BackUp(5);
    CHECK(buffer.ByteCount() == 5);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<char*>(data) == data1 + 5);
    CHECK(size == 5);
  }

  SECTION("BackUp to the second buffer returns the correct partial buffer") {
    char data1[10];
    char data2[5];
    BipartMemoryOutputStream buffer{data1, 10, data2, 5};
    void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    CHECK(buffer.Next(&data, &size));
    buffer.BackUp(3);
    CHECK(buffer.ByteCount() == 12);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<char*>(data) == data2 + 2);
    CHECK(size == 3);
  }
}

TEST_CASE("BipartMemoryInputStream") {
  SECTION("Next returns false when there's no data attached.") {
    BipartMemoryInputStream buffer{nullptr, 0, nullptr, 0};
    const void* data;
    int size;
    CHECK(!buffer.Next(&data, &size));
  }

  SECTION("Next returns the first memory buffer provided") {
    char data1[10];
    BipartMemoryInputStream buffer{data1, 10, nullptr, 0};
    const void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<const char*>(data) == data1);
    CHECK(size == 10);
    CHECK(buffer.ByteCount() == 10);
    CHECK(!buffer.Next(&data, &size));
  }

  SECTION("Next returns the first, then second memory buffer if provided") {
    char data1[10];
    char data2[5];
    BipartMemoryInputStream buffer{data1, 10, data2, 5};
    const void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<const char*>(data) == data1);
    CHECK(size == 10);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<const char*>(data) == data2);
    CHECK(size == 5);
    CHECK(buffer.ByteCount() == 15);
    CHECK(!buffer.Next(&data, &size));
  }

  SECTION("BackUp to the first buffer returns thre correct partial buffer") {
    char data1[10];
    char data2[5];
    BipartMemoryInputStream buffer{data1, 10, data2, 5};
    const void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    buffer.BackUp(5);
    CHECK(buffer.ByteCount() == 5);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<const char*>(data) == data1 + 5);
    CHECK(size == 5);
  }

  SECTION("BackUp to the second buffer returns the correct partial buffer") {
    char data1[10];
    char data2[5];
    BipartMemoryInputStream buffer{data1, 10, data2, 5};
    const void* data;
    int size;
    CHECK(buffer.Next(&data, &size));
    CHECK(buffer.Next(&data, &size));
    buffer.BackUp(3);
    CHECK(buffer.ByteCount() == 12);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<const char*>(data) == data2 + 2);
    CHECK(size == 3);
  }

  SECTION("Skip acts as if bytes are read") {
    char data1[10];
    BipartMemoryInputStream buffer{data1, 10, nullptr, 0};
    CHECK(buffer.Skip(2));
    const void* data;
    int size;
    CHECK(buffer.ByteCount() == 2);
    CHECK(buffer.Next(&data, &size));
    CHECK(static_cast<const char*>(data) == data1 + 2);
    CHECK(size == 8);
  }

  SECTION("Skipping more bytes than are in the buffer returns false") {
    char data1[10];
    BipartMemoryInputStream buffer{data1, 10, nullptr, 0};
    CHECK(!buffer.Skip(11));
  }
}
