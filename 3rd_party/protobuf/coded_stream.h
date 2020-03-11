#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

#include "3rd_party/protobuf/zero_copy_stream.h"

namespace lightstep {
using uint8 = unsigned char;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

class EpsCopyOutputStream {
 public:
  enum { kSlopBytes = 16 };

  // Initialize from a stream.
  EpsCopyOutputStream(ZeroCopyOutputStream* stream, bool deterministic,
                      uint8** pp)
      : end_(buffer_),
        stream_(stream),
        is_serialization_deterministic_(deterministic) {
    *pp = buffer_;
  }

  // Only for array serialization. No overflow protection, end_ will be the
  // pointed to the end of the array. When using this the total size is already
  // known, so no need to maintain the slop region.
  EpsCopyOutputStream(void* data, int size, bool deterministic)
      : end_(static_cast<uint8*>(data) + size),
        buffer_end_(nullptr),
        stream_(nullptr),
        is_serialization_deterministic_(deterministic) {}

  // Initialize from stream but with the first buffer already given (eager).
  EpsCopyOutputStream(void* data, int size, ZeroCopyOutputStream* stream,
                      bool deterministic, uint8** pp)
      : stream_(stream), is_serialization_deterministic_(deterministic) {
    *pp = SetInitialBuffer(data, size);
  }

  // Flush everything that's written into the underlying ZeroCopyOutputStream
  // and trims the underlying stream to the location of ptr.
  uint8* Trim(uint8* ptr);

  // After this it's guaranteed you can safely write kSlopBytes to ptr. This
  // will never fail! The underlying stream can produce an error. Use HadError
  // to check for errors.
  uint8* EnsureSpace(uint8* ptr) {
    if (PROTOBUF_PREDICT_FALSE(ptr >= end_)) {
      return EnsureSpaceFallback(ptr);
    }
    return ptr;
  }

  uint8* WriteRaw(const void* data, int size, uint8* ptr) {
    if (PROTOBUF_PREDICT_FALSE(end_ - ptr < size)) {
      return WriteRawFallback(data, size, ptr);
    }
    std::memcpy(ptr, data, size);
    return ptr + size;
  }
  // Writes the buffer specified by data, size to the stream. Possibly by
  // aliasing the buffer (ie. not copying the data). The caller is responsible
  // to make sure the buffer is alive for the duration of the
  // ZeroCopyOutputStream.
  uint8* WriteRawMaybeAliased(const void* data, int size, uint8* ptr) {
    if (aliasing_enabled_) {
      return WriteAliasedRaw(data, size, ptr);
    } else {
      return WriteRaw(data, size, ptr);
    }
  }


  uint8* WriteStringMaybeAliased(uint32 num, const std::string& s, uint8* ptr) {
    std::ptrdiff_t size = s.size();
    if (PROTOBUF_PREDICT_FALSE(
            size >= 128 || end_ - ptr + 16 - TagSize(num << 3) - 1 < size)) {
      return WriteStringMaybeAliasedOutline(num, s, ptr);
    }
    ptr = UnsafeVarint((num << 3) | 2, ptr);
    *ptr++ = static_cast<uint8>(size);
    std::memcpy(ptr, s.data(), size);
    return ptr + size;
  }
  uint8* WriteBytesMaybeAliased(uint32 num, const std::string& s, uint8* ptr) {
    return WriteStringMaybeAliased(num, s, ptr);
  }

  template <typename T>
  inline uint8* WriteString(uint32 num, const T& s,
                                            uint8* ptr) {
    std::ptrdiff_t size = s.size();
    if (PROTOBUF_PREDICT_FALSE(
            size >= 128 || end_ - ptr + 16 - TagSize(num << 3) - 1 < size)) {
      return WriteStringOutline(num, s, ptr);
    }
    ptr = UnsafeVarint((num << 3) | 2, ptr);
    *ptr++ = static_cast<uint8>(size);
    std::memcpy(ptr, s.data(), size);
    return ptr + size;
  }
  template <typename T>
  uint8* WriteBytes(uint32 num, const T& s, uint8* ptr) {
    return WriteString(num, s, ptr);
  }

  template <typename T>
  inline uint8* WriteInt32Packed(int num, const T& r, int size,
                                                 uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, Encode64);
  }
  template <typename T>
  inline uint8* WriteUInt32Packed(int num, const T& r, int size,
                                                  uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, Encode32);
  }
  template <typename T>
  inline uint8* WriteSInt32Packed(int num, const T& r, int size,
                                                  uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, ZigZagEncode32);
  }
  template <typename T>
  inline uint8* WriteInt64Packed(int num, const T& r, int size,
                                                 uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, Encode64);
  }
  template <typename T>
  inline uint8* WriteUInt64Packed(int num, const T& r, int size,
                                                  uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, Encode64);
  }
  template <typename T>
  inline uint8* WriteSInt64Packed(int num, const T& r, int size,
                                                  uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, ZigZagEncode64);
  }
  template <typename T>
  inline uint8* WriteEnumPacked(int num, const T& r, int size,
                                                uint8* ptr) {
    return WriteVarintPacked(num, r, size, ptr, Encode64);
  }

  template <typename T>
  inline uint8* WriteFixedPacked(int num, const T& r,
                                                 uint8* ptr) {
    ptr = EnsureSpace(ptr);
    constexpr auto element_size = sizeof(typename T::value_type);
    auto size = r.size() * element_size;
    ptr = WriteLengthDelim(num, size, ptr);
    return WriteRawLittleEndian<element_size>(r.data(), static_cast<int>(size),
                                              ptr);
  }

  // Returns true if there was an underlying I/O error since this object was
  // created.
  bool HadError() const { return had_error_; }

  // Instructs the EpsCopyOutputStream to allow the underlying
  // ZeroCopyOutputStream to hold pointers to the original structure instead of
  // copying, if it supports it (i.e. output->AllowsAliasing() is true).  If the
  // underlying stream does not support aliasing, then enabling it has no
  // affect.  For now, this only affects the behavior of
  // WriteRawMaybeAliased().
  //
  // NOTE: It is caller's responsibility to ensure that the chunk of memory
  // remains live until all of the data has been consumed from the stream.
  void EnableAliasing(bool enabled);

  // See documentation on CodedOutputStream::SetSerializationDeterministic.
  void SetSerializationDeterministic(bool value) {
    is_serialization_deterministic_ = value;
  }

  // See documentation on CodedOutputStream::IsSerializationDeterministic.
  bool IsSerializationDeterministic() const {
    return is_serialization_deterministic_;
  }

  // The number of bytes written to the stream at position ptr, relative to the
  // stream's overall position.
  int64 ByteCount(uint8* ptr) const;


 private:
  uint8* end_;
  uint8* buffer_end_ = buffer_;
  uint8 buffer_[2 * kSlopBytes];
  ZeroCopyOutputStream* stream_;
  bool had_error_ = false;
  bool aliasing_enabled_ = false;  // See EnableAliasing().
  bool is_serialization_deterministic_;

  uint8* EnsureSpaceFallback(uint8* ptr);
  inline uint8* Next();
  int Flush(uint8* ptr);
  std::ptrdiff_t GetSize(uint8* ptr) const {
    assert(ptr <= end_ + kSlopBytes);  // NOLINT
    return end_ + kSlopBytes - ptr;
  }

  uint8* Error() {
    had_error_ = true;
    // We use the patch buffer to always guarantee space to write to.
    end_ = buffer_ + kSlopBytes;
    return buffer_;
  }

  static constexpr int TagSize(uint32 tag) {
    return (tag < (1 << 7))
               ? 1
               : (tag < (1 << 14))
                     ? 2
                     : (tag < (1 << 21)) ? 3 : (tag < (1 << 28)) ? 4 : 5;
  }

  inline uint8* WriteTag(uint32 num, uint32 wt, uint8* ptr) {
    assert(ptr < end_);  // NOLINT
    return UnsafeVarint((num << 3) | wt, ptr);
  }

  inline uint8* WriteLengthDelim(int num, uint32 size,
                                                 uint8* ptr) {
    ptr = WriteTag(num, 2, ptr);
    return UnsafeWriteSize(size, ptr);
  }

  uint8* WriteRawFallback(const void* data, int size, uint8* ptr);

  uint8* WriteAliasedRaw(const void* data, int size, uint8* ptr);

  uint8* WriteStringMaybeAliasedOutline(uint32 num, const std::string& s,
                                        uint8* ptr);
  uint8* WriteStringOutline(uint32 num, const std::string& s, uint8* ptr);

  template <typename T, typename E>
  inline uint8* WriteVarintPacked(int num, const T& r, int size,
                                                  uint8* ptr, const E& encode) {
    ptr = EnsureSpace(ptr);
    ptr = WriteLengthDelim(num, size, ptr);
    auto it = r.data();
    auto end = it + r.size();
    do {
      ptr = EnsureSpace(ptr);
      ptr = UnsafeVarint(encode(*it++), ptr);
    } while (it < end);
    return ptr;
  }

  static uint32 Encode32(uint32 v) { return v; }
  static uint64 Encode64(uint64 v) { return v; }
  static uint32 ZigZagEncode32(int32 v) {
    return (static_cast<uint32>(v) << 1) ^ static_cast<uint32>(v >> 31);
  }
  static uint64 ZigZagEncode64(int64 v) {
    return (static_cast<uint64>(v) << 1) ^ static_cast<uint64>(v >> 63);
  }

  template <typename T>
  inline static uint8* UnsafeVarint(T value, uint8* ptr) {
    static_assert(std::is_unsigned<T>::value,
                  "Varint serialization must be unsigned");
    if (value < 0x80) {
      ptr[0] = static_cast<uint8>(value);
      return ptr + 1;
    }
    ptr[0] = static_cast<uint8>(value | 0x80);
    value >>= 7;
    if (value < 0x80) {
      ptr[1] = static_cast<uint8>(value);
      return ptr + 2;
    }
    ptr++;
    do {
      *ptr = static_cast<uint8>(value | 0x80);
      value >>= 7;
      ++ptr;
    } while (PROTOBUF_PREDICT_FALSE(value >= 0x80));
    *ptr++ = static_cast<uint8>(value);
    return ptr;
  }

  inline static uint8* UnsafeWriteSize(uint32 value,
                                                       uint8* ptr) {
    while (PROTOBUF_PREDICT_FALSE(value >= 0x80)) {
      *ptr = static_cast<uint8>(value | 0x80);
      value >>= 7;
      ++ptr;
    }
    *ptr++ = static_cast<uint8>(value);
    return ptr;
  }

  // These methods are for CodedOutputStream. Ideally they should be private
  // but to match current behavior of CodedOutputStream as close as possible
  // we allow it some functionality.
 public:
  uint8* SetInitialBuffer(void* data, int size) {
    auto ptr = static_cast<uint8*>(data);
    if (size > kSlopBytes) {
      end_ = ptr + size - kSlopBytes;
      buffer_end_ = nullptr;
      return ptr;
    } else {
      end_ = buffer_ + size;
      buffer_end_ = ptr;
      return buffer_;
    }
  }

 private:
  // Needed by CodedOutputStream HadError. HadError needs to flush the patch
  // buffers to ensure there is no error as of yet.
  uint8* FlushAndResetBuffer(uint8*);

  // The following functions mimick the old CodedOutputStream behavior as close
  // as possible. They flush the current state to the stream, behave as
  // the old CodedOutputStream and then return to normal operation.
  bool Skip(int count, uint8** pp);
  bool GetDirectBufferPointer(void** data, int* size, uint8** pp);
  uint8* GetDirectBufferForNBytesAndAdvance(int size, uint8** pp);

  friend class CodedOutputStream;
};

#if 0
class CodedOutputStream {
 public:
  // Create an CodedOutputStream that writes to the given ZeroCopyOutputStream.
  explicit CodedOutputStream(ZeroCopyOutputStream* stream)
      : CodedOutputStream(stream, true) {}

  CodedOutputStream(ZeroCopyOutputStream* stream, bool do_eager_refresh);

  // Destroy the CodedOutputStream and position the underlying
  // ZeroCopyOutputStream immediately after the last byte written.
  ~CodedOutputStream();

  // Returns true if there was an underlying I/O error since this object was
  // created. On should call Trim before this function in order to catch all
  // errors.
  bool HadError() {
    cur_ = impl_.FlushAndResetBuffer(cur_);
    assert(cur_);
    return impl_.HadError();
  }

  // Trims any unused space in the underlying buffer so that its size matches
  // the number of bytes written by this stream. The underlying buffer will
  // automatically be trimmed when this stream is destroyed; this call is only
  // necessary if the underlying buffer is accessed *before* the stream is
  // destroyed.
  void Trim() { cur_ = impl_.Trim(cur_); }

  // Skips a number of bytes, leaving the bytes unmodified in the underlying
  // buffer.  Returns false if an underlying write error occurs.  This is
  // mainly useful with GetDirectBufferPointer().
  // Note of caution, the skipped bytes may contain uninitialized data. The
  // caller must make sure that the skipped bytes are properly initialized,
  // otherwise you might leak bytes from your heap.
  bool Skip(int count) { return impl_.Skip(count, &cur_); }

  // Sets *data to point directly at the unwritten part of the
  // CodedOutputStream's underlying buffer, and *size to the size of that
  // buffer, but does not advance the stream's current position.  This will
  // always either produce a non-empty buffer or return false.  If the caller
  // writes any data to this buffer, it should then call Skip() to skip over
  // the consumed bytes.  This may be useful for implementing external fast
  // serialization routines for types of data not covered by the
  // CodedOutputStream interface.
  bool GetDirectBufferPointer(void** data, int* size) {
    return impl_.GetDirectBufferPointer(data, size, &cur_);
  }

  // If there are at least "size" bytes available in the current buffer,
  // returns a pointer directly into the buffer and advances over these bytes.
  // The caller may then write directly into this buffer (e.g. using the
  // *ToArray static methods) rather than go through CodedOutputStream.  If
  // there are not enough bytes available, returns NULL.  The return pointer is
  // invalidated as soon as any other non-const method of CodedOutputStream
  // is called.
  inline uint8* GetDirectBufferForNBytesAndAdvance(int size) {
    return impl_.GetDirectBufferForNBytesAndAdvance(size, &cur_);
  }

  // Write raw bytes, copying them from the given buffer.
  void WriteRaw(const void* buffer, int size) {
    cur_ = impl_.WriteRaw(buffer, size, cur_);
  }

  static uint8* WriteRawToArray(const void* buffer, int size, uint8* target);

  // Equivalent to WriteRaw(str.data(), str.size()).
  void WriteString(const std::string& str);
  // Like WriteString()  but writing directly to the target array.
  static uint8* WriteStringToArray(const std::string& str, uint8* target);
  // Write the varint-encoded size of str followed by str.
  static uint8* WriteStringWithSizeToArray(const std::string& str,
                                           uint8* target);

  // Write an unsigned integer with Varint encoding.  Writing a 32-bit value
  // is equivalent to casting it to uint64 and writing it as a 64-bit value,
  // but may be more efficient.
  void WriteVarint32(uint32 value);
  // Like WriteVarint32()  but writing directly to the target array.
  static uint8* WriteVarint32ToArray(uint32 value, uint8* target);
  // Write an unsigned integer with Varint encoding.
  void WriteVarint64(uint64 value);
  // Like WriteVarint64()  but writing directly to the target array.
  static uint8* WriteVarint64ToArray(uint64 value, uint8* target);

  // This is identical to WriteVarint32(), but optimized for writing tags.
  // In particular, if the input is a compile-time constant, this method
  // compiles down to a couple instructions.
  // Always inline because otherwise the aformentioned optimization can't work,
  // but GCC by default doesn't want to inline this.
  void WriteTag(uint32 value);
  // Like WriteTag()  but writing directly to the target array.
  PROTOBUF_ALWAYS_INLINE
  static uint8* WriteTagToArray(uint32 value, uint8* target);

  // Returns the number of bytes needed to encode the given value as a varint.
  static size_t VarintSize32(uint32 value);
  // Returns the number of bytes needed to encode the given value as a varint.
  static size_t VarintSize64(uint64 value);

  // If negative, 10 bytes.  Otherwise, same as VarintSize32().
  static size_t VarintSize32SignExtended(int32 value);

  // Compile-time equivalent of VarintSize32().
  template <uint32 Value>
  struct StaticVarintSize32 {
    static const size_t value =
        (Value < (1 << 7))
            ? 1
            : (Value < (1 << 14))
                  ? 2
                  : (Value < (1 << 21)) ? 3 : (Value < (1 << 28)) ? 4 : 5;
  };

  // Returns the total number of bytes written since this object was created.
  int ByteCount() const {
    return static_cast<int>(impl_.ByteCount(cur_) - start_count_);
  }

  // Instructs the CodedOutputStream to allow the underlying
  // ZeroCopyOutputStream to hold pointers to the original structure instead of
  // copying, if it supports it (i.e. output->AllowsAliasing() is true).  If the
  // underlying stream does not support aliasing, then enabling it has no
  // affect.  For now, this only affects the behavior of
  // WriteRawMaybeAliased().
  //
  // NOTE: It is caller's responsibility to ensure that the chunk of memory
  // remains live until all of the data has been consumed from the stream.
  void EnableAliasing(bool enabled) { impl_.EnableAliasing(enabled); }

  // Indicate to the serializer whether the user wants derministic
  // serialization. The default when this is not called comes from the global
  // default, controlled by SetDefaultSerializationDeterministic.
  //
  // What deterministic serialization means is entirely up to the driver of the
  // serialization process (i.e. the caller of methods like WriteVarint32). In
  // the case of serializing a proto buffer message using one of the methods of
  // MessageLite, this means that for a given binary equal messages will always
  // be serialized to the same bytes. This implies:
  //
  //   * Repeated serialization of a message will return the same bytes.
  //
  //   * Different processes running the same binary (including on different
  //     machines) will serialize equal messages to the same bytes.
  //
  // Note that this is *not* canonical across languages. It is also unstable
  // across different builds with intervening message definition changes, due to
  // unknown fields. Users who need canonical serialization (e.g. persistent
  // storage in a canonical form, fingerprinting) should define their own
  // canonicalization specification and implement the serializer using
  // reflection APIs rather than relying on this API.
  void SetSerializationDeterministic(bool value) {
    impl_.SetSerializationDeterministic(value);
  }

  // Return whether the user wants deterministic serialization. See above.
  bool IsSerializationDeterministic() const {
    return impl_.IsSerializationDeterministic();
  }

  static bool IsDefaultSerializationDeterministic() {
    return default_serialization_deterministic_.load(
               std::memory_order_relaxed) != 0;
  }

  uint8* Cur() const { return cur_; }
  void SetCur(uint8* ptr) { cur_ = ptr; }
  EpsCopyOutputStream* EpsCopy() { return &impl_; }

 private:
  EpsCopyOutputStream impl_;
  uint8* cur_;
  int64 start_count_;
  static std::atomic<bool> default_serialization_deterministic_;

  // See above.  Other projects may use "friend" to allow them to call this.
  // After SetDefaultSerializationDeterministic() completes, all protocol
  // buffer serializations will be deterministic by default.  Thread safe.
  // However, the meaning of "after" is subtle here: to be safe, each thread
  // that wants deterministic serialization by default needs to call
  // SetDefaultSerializationDeterministic() or ensure on its own that another
  // thread has done so.
  friend void internal::MapTestForceDeterministic();
  static void SetDefaultSerializationDeterministic() {
    default_serialization_deterministic_.store(true, std::memory_order_relaxed);
  }
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CodedOutputStream);
};
#endif
}  // namespace lightstep
