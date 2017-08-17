set(CMAKE_CXX_STANDARD 11)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(WARNING_CXX_FLAGS -Weverything 
    -Wno-c++98-compat 
    -Wno-c++98-compat-bind-to-temporary-copy 
    -Wno-weak-vtables 
    -Wno-exit-time-destructors 
    -Wno-global-constructors 
    -Wno-sign-conversion
    -Wno-padded)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(WARNING_CXX_FLAGS -Wall -Wextra)
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy" 
                            DOC "Path to clang-tidy executable")
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*\
,-llvm-include-order,-google-runtime-references,-google-build-using-namespace,-cppcoreguidelines-pro-type-vararg")
endif()

configure_file(version.h.in include/lightstep/version.h)
configure_file(config.h.in include/lightstep/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lightstep 
        DESTINATION include)

include_directories(SYSTEM 3rd_party/include)

# Needed to make some of the internal relative includes work.
include_directories(SYSTEM 3rd_party/include/lightstep)

include_directories(include)
install(DIRECTORY include/lightstep DESTINATION include)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(LightStepProtobuf)

set(LIGHTSTEP_SRCS src/utility.cpp
                   src/propagation.cpp
                   src/binary_carrier.cpp
                   src/grpc_transporter.cpp
                   src/nghttp2_async_transporter.cpp
                   src/report_builder.cpp
                   src/manual_recorder.cpp
                   src/buffered_recorder.cpp
                   src/lightstep_span_context.cpp
                   src/lightstep_span.cpp
                   src/lightstep_tracer_impl.cpp
                   src/transporter.cpp
                   src/tracer.cpp)

add_library(lightstep_tracer $<TARGET_OBJECTS:lightstep_protobuf>
                             ${LIGHTSTEP_SRCS})
target_compile_options(lightstep_tracer PUBLIC ${WARNING_CXX_FLAGS})
if (CLANG_TIDY_EXE)
  set_target_properties(lightstep_tracer PROPERTIES
                                         CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

install(TARGETS lightstep_tracer
        LIBRARY DESTINATION lib 
        ARCHIVE DESTINATION lib)

target_link_libraries(lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})

if (WITH_ASAN)
  add_library(lightstep_tracer_asan $<TARGET_OBJECTS:lightstep_protobuf>
                                    ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer_asan PUBLIC ${ASAN_CXX_FLAGS})
  target_link_libraries(lightstep_tracer_asan ${LIGHTSTEP_LINK_LIBRARIES})
  set_target_properties(lightstep_tracer_asan 
    PROPERTIES LINK_FLAGS "${ASAN_LD_FLAGS}")
endif()

if (WITH_TSAN)
  add_library(lightstep_tracer_tsan $<TARGET_OBJECTS:lightstep_protobuf>
                                    ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer_tsan PUBLIC ${TSAN_CXX_FLAGS})
  target_link_libraries(lightstep_tracer_tsan ${LIGHTSTEP_LINK_LIBRARIES})
  set_target_properties(lightstep_tracer_tsan 
    PROPERTIES LINK_FLAGS "${TSAN_LD_FLAGS}")
endif()

add_subdirectory(test)
add_subdirectory(example)
