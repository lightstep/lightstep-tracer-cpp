#include <opentracing/version.h>

static_assert(OPENTRACING_ABI_VERSION[0] == '2',
    "Invalid version of OpenTracing. Please install a version using the v2 ABI.");
