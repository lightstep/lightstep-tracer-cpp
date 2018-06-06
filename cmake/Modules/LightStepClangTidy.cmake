find_program(CLANG_TIDY_EXE NAMES "clang-tidy" "clang-tidy-5.0" "clang-tidy-6.0" "clang-tidy-7.0"
                            DOC "Path to clang-tidy executable")
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" 
"-checks=*,\
-fuchsia-*,\
-clang-analyzer-alpha.*,\
-llvm-include-order,\
-google-runtime-references,\
-google-build-using-namespace,\
-modernize-make-unique,\
-hicpp-vararg,\
-cppcoreguidelines-owning-memory,\
-cppcoreguidelines-pro-type-reinterpret-cast,\
-cppcoreguidelines-pro-type-const-cast,\
-cppcoreguidelines-pro-type-vararg;\
-warnings-as-errors=*")
endif()

macro(_apply_clang_tidy_if_available TARGET)
  if (CLANG_TIDY_EXE)
    set_target_properties(${TARGET} PROPERTIES
                                           CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
  endif()
endmacro()
