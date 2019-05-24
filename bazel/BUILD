package(default_visibility = ["//visibility:public"])

exports_files(['sanitize-blacklist.txt'])

config_setting(
    name = "asan_build",
    values = {"define": "LIGHTSTEP_CONFIG_ASAN=1"},
)

config_setting(
    name = "tsan_build",
    values = {"define": "LIGHTSTEP_CONFIG_TSAN=1"},
)
