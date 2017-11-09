proto_library(
    name = "collector_proto",
    srcs = ["collector.proto"],
    deps = [
        "@lightstep_vendored_googleapis//:googleapis_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "collector_proto_cc",
    deps = [":collector_proto"],
)

proto_library(
    name = "lightstep_carrier_proto",
    srcs = ["lightstep_carrier.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "lightstep_carrier_proto_cc",
    deps = [":lightstep_carrier_proto"],
)

# FIXME: this rule allows `#include <collector.pb.h>` to work, but it
# would be nicer to have properly prefixed include paths instead.
cc_library(
    name = "lightstep_protos",
    deps = [
        ":collector_proto_cc",
        ":lightstep_carrier_proto_cc",
    ],
    visibility = ["//visibility:public"],
    includes = ["."]
)
