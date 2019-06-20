#include <cstring>
#include <ctime>

extern "C" {
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
__asm__(".symver clock_gettime,clock_gettime@GLIBC_2.2.5");

void* __wrap_memcpy(void* dest, const void* src, size_t n) {
  return memcpy(dest, src, n);
}

int __wrap_clock_gettime(clockid_t clk_id, struct timespec* tp) {
  return clock_gettime(clk_id, tp);
}
}  // extern "C"
