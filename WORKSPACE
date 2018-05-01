workspace(name = "com_lightstep_tracer_cpp")

git_repository(
    name = "io_opentracing_cpp",
    remote = "https://github.com/opentracing/opentracing-cpp",
    commit = "900f9d9297a71ddf4a5dff2051a01493014c07c5",
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "5d4551193416861cb81c3bc0a428f22a6878148c57c31fb6f8f2aa4cf27ff635",
    strip_prefix = "protobuf-c4f59dcc5c13debc572154c8f636b8a9361aacde",
    urls = ["https://github.com/google/protobuf/archive/c4f59dcc5c13debc572154c8f636b8a9361aacde.tar.gz"],
)

http_archive(
    name = "com_google_protobuf_cc",
    sha256 = "5d4551193416861cb81c3bc0a428f22a6878148c57c31fb6f8f2aa4cf27ff635",
    strip_prefix = "protobuf-c4f59dcc5c13debc572154c8f636b8a9361aacde",
    urls = ["https://github.com/google/protobuf/archive/c4f59dcc5c13debc572154c8f636b8a9361aacde.tar.gz"],
)

local_repository(
    name = "lightstep_vendored_googleapis",
    path = "lightstep-tracer-common/third_party/googleapis",
)
