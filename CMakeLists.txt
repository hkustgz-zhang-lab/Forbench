cmake_minimum_required(VERSION 3.1)

project(wasim)

set(WASIM_MAJOR 0)   # Major component of WASIM version
set(WASIM_MINOR 1)   # Minor component of WASIM version
set(WASIM_RELEASE 1) # Release component of WASIM version

# handle different versions of CMake
if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0 AND NOT APPLE)
  set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std=c++17")
  set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std=gnu++17")
elseif (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1 OR APPLE)
  set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std=c++1z")
  set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std=gnu++1z")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DWASIM_SRC_DIR=${PROJECT_SOURCE_DIR})

if(APPLE)
  set(SHARED_LIB_EXT "dylib")
else()
  set(SHARED_LIB_EXT "so")
endif()



if (WITH_PROFILING)
  find_library(GOOGLE_PERF profiler REQUIRED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_PROFILING")
endif()

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/deps/bison/bison-install")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/deps/flex/flex-install")
if (APPLE)
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/bison")
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/flex")
endif()
find_package(BISON 3.4.2 REQUIRED)
find_package(FLEX 2.6.4 REQUIRED)

if (BISON_FOUND)
  get_filename_component(BISON_PARENT_DIR "${BISON_EXECUTABLE}" DIRECTORY)
  message("-- Adding bison lib: ${BISON_PARENT_DIR}/../lib")
  link_directories("${BISON_PARENT_DIR}/../lib/")
endif()

find_library(LIBRT rt)

message("-- FOUND FLEX EXECUTABLE: ${FLEX_EXECUTABLE}")
message("-- FOUND FLEX INCLUDE DIRS: ${FLEX_INCLUDE_DIRS}")


find_package(Boost REQUIRED COMPONENTS system filesystem)
MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
MESSAGE( STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}.")
MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")

# TODO: Use find_package
# find package
#list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/smt-switch")

set(SMT_SWITCH_DIR "${PROJECT_SOURCE_DIR}/deps/smt-switch")

# rely on cmake modules from smt-switch (required anyway)
set(CMAKE_MODULE_PATH ${SMT_SWITCH_DIR}/cmake)

find_package(GMP REQUIRED)

# Check that dependencies are there
if (NOT EXISTS "${SMT_SWITCH_DIR}/local/include/smt-switch/smt.h")
  message(FATAL_ERROR "Missing smt-switch headers -- try running ./contrib/setup-smt-switch.sh")
endif()

if (NOT EXISTS "${SMT_SWITCH_DIR}/local/lib/libsmt-switch.a")
  message(FATAL_ERROR "Missing smt-switch library -- try running ./contrib/setup-smt-switch.sh")
endif()

if (NOT EXISTS "${SMT_SWITCH_DIR}/local/lib/libsmt-switch-btor.a")
  message(FATAL_ERROR "Missing smt-switch boolector library -- try running ./contrib/setup-smt-switch.sh")
endif()



if (NOT EXISTS "${PROJECT_SOURCE_DIR}/deps/btor2tools/src")
  message(FATAL_ERROR "Missing btor2tools -- try running ./contrib/setup-btor2tools.sh")
endif()


if (NOT EXISTS "${PROJECT_SOURCE_DIR}/deps/glog/install/lib/libglog.a")
  message(FATAL_ERROR "Missing glog -- try running ./contrib/setup-glog.sh")
endif()

# Generate the SMV parser using Flex/Bison
BISON_TARGET(SMVParser ${PROJECT_SOURCE_DIR}/frontends/smvparser.y
  ${CMAKE_CURRENT_BINARY_DIR}/smvparser.cpp
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/smvparser.h)

FLEX_TARGET(SMVScanner ${PROJECT_SOURCE_DIR}/frontends/smvparser.l
  ${CMAKE_CURRENT_BINARY_DIR}/smvlexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(SMVScanner SMVParser)

set(INCLUDE_DIRS
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/contrib/"
  "${PROJECT_SOURCE_DIR}/contrib/optionparser-1.7/src"
  "${PROJECT_SOURCE_DIR}/deps/btor2tools/src"
  "${PROJECT_SOURCE_DIR}/deps/glog/install/include/"
  "${PROJECT_SOURCE_DIR}/deps/vexpparser/include/"
  "${PROJECT_SOURCE_DIR}/deps/vexpparser/src"
  "${PROJECT_SOURCE_DIR}/deps/vexpparser/build/src"
  "${SMT_SWITCH_DIR}/local/include"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${GMP_INCLUDE_DIR}"
  "${FLEX_INCLUDE_DIRS}"
  "${Boost_INCLUDE_DIRS}"
  )

set(SOURCES
    "${PROJECT_SOURCE_DIR}/frontend/btor2_encoder.cpp"
    "${PROJECT_SOURCE_DIR}/frontend/smt_in.cpp"
    "${PROJECT_SOURCE_DIR}/frontend/state_read_write.cpp"
    "${PROJECT_SOURCE_DIR}/framework/ts.cpp"
    "${PROJECT_SOURCE_DIR}/framework/symsim.cpp"
    "${PROJECT_SOURCE_DIR}/framework/symsim_branch.cpp"
    "${PROJECT_SOURCE_DIR}/framework/independence_check.cpp"
    "${PROJECT_SOURCE_DIR}/framework/tracemgr.cpp"
    "${PROJECT_SOURCE_DIR}/framework/symtraverse.cpp"
    "${PROJECT_SOURCE_DIR}/framework/term_manip.cpp"
    "${PROJECT_SOURCE_DIR}/framework/sygus_simplify.cpp"
    "${PROJECT_SOURCE_DIR}/framework/state_simplify.cpp"
    "${PROJECT_SOURCE_DIR}/utils/logger.cpp"
    "${PROJECT_SOURCE_DIR}/timed-assertion-checker/timed_assertion_checker.cpp"
)




add_library(wasim-lib "${WASIM_LIB_TYPE}" ${SOURCES})
set_target_properties(wasim-lib PROPERTIES OUTPUT_NAME wasim)

target_include_directories(wasim-lib PUBLIC ${INCLUDE_DIRS})

target_link_libraries(wasim-lib PUBLIC "${PROJECT_SOURCE_DIR}/deps/glog/install/lib/libglog.a")
target_link_libraries(wasim-lib PUBLIC "${SMT_SWITCH_DIR}/local/lib/libsmt-switch-btor.a")
target_link_libraries(wasim-lib PUBLIC "${SMT_SWITCH_DIR}/local/lib/libsmt-switch-cvc5.a")
target_link_libraries(wasim-lib PUBLIC "${SMT_SWITCH_DIR}/local/lib/libsmt-switch.a")
target_link_libraries(wasim-lib PUBLIC "${PROJECT_SOURCE_DIR}/deps/btor2tools/build/lib/libbtor2parser.a")
target_link_libraries(wasim-lib PUBLIC "${PROJECT_SOURCE_DIR}/deps/vexpparser/build/src/libvexpparser.a")
target_link_libraries(wasim-lib PUBLIC "${GMPXX_LIBRARIES}")
target_link_libraries(wasim-lib PUBLIC "${GMP_LIBRARIES}")
target_link_libraries(wasim-lib PUBLIC pthread)
target_link_libraries(wasim-lib PUBLIC Boost::system)
# if (LIBRT)
#   target_link_libraries(wasim-lib PUBLIC ${LIBRT})
# endif()

# if (GOOGLE_PERF)
#   target_link_libraries(wasim-lib PUBLIC ${GOOGLE_PERF})
# endif()

# enable_testing()
# Add tests subdirectory
# The CMakeLists.txt file there sets up googletest
# and builds all the parametrized tests
# add_subdirectory(tests)

#add_executable(wasim-bin "${PROJECT_SOURCE_DIR}/inv_check1.cpp")
#set_target_properties(wasim-bin PROPERTIES OUTPUT_NAME wasim)


# target_include_directories(wasim-bin PUBLIC
#   "${PROJECT_SOURCE_DIR}/utils/"
#   "${PROJECT_SOURCE_DIR}/contrib/"
#   "${PROJECT_SOURCE_DIR}/contrib/optionparser-1.7/src"
#   "${PROJECT_SOURCE_DIR}/deps/btor2tools/src"
#   "${SMT_SWITCH_DIR}/local/include")

# target_link_libraries(wasim-bin PUBLIC wasim-lib)

# if ("${WASIM_STATIC_EXEC}" STREQUAL "YES")
#   target_link_libraries(wasim-bin PUBLIC -static)
# endif()

# install(TARGETS wasim-bin DESTINATION bin)


# Should we build python bindings
option (BUILD_PYTHON_BINDINGS
   "Build Python bindings")

if (BUILD_PYTHON_BINDINGS)

    find_package(Python REQUIRED COMPONENTS Interpreter Development)
    find_package(Boost REQUIRED COMPONENTS python3 REQUIRED)

    message( STATUS "Python_FOUND:${Python_FOUND}")
    message( STATUS "Python_VERSION:${Python_VERSION}")
    message( STATUS "Python_Development_FOUND:${Python_Development_FOUND}")
    message( STATUS "Python_LIBRARIES:${Python_LIBRARIES}")
    message( STATUS "Python_INCLUDE_DIRS:${Python_INCLUDE_DIRS}")
    message( STATUS "Python_SITEARCH:${Python_SITEARCH}")
    message( STATUS "Boost_LIBRARIES:${Boost_LIBRARIES}")

    add_library(pywasimbase MODULE python/pywasimbase.cpp)
    target_link_libraries(pywasimbase ${Boost_LIBRARIES} ${Python_LIBRARIES})
    target_link_libraries(pywasimbase ${LIBRT})
    target_link_libraries(pywasimbase wasim-lib)
    target_include_directories(pywasimbase PRIVATE ${INCLUDE_DIRS})
    target_include_directories(pywasimbase PRIVATE ${Python_INCLUDE_DIRS})
    
    configure_file(python/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)
    set_target_properties(pywasimbase PROPERTIES PREFIX "")

    install(TARGETS pywasimbase DESTINATION "${Python_SITEARCH}")
    install(FILES python/__init__.py DESTINATION "${Python_SITEARCH}")

endif()

# install smt-switch
install(TARGETS wasim-lib DESTINATION lib)

# install public headers
install(DIRECTORY "${PROJECT_SOURCE_DIR}/framework/"
        DESTINATION include/wasim/framework
        FILES_MATCHING PATTERN "*.h")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/utils/"
        DESTINATION include/wasim/utils
        FILES_MATCHING PATTERN "*.h")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/contrib/fmt/"
        DESTINATION include/wasim/fmt
        FILES_MATCHING PATTERN "*.h")

# uninstall target
# copied from https://gitlab.kitware.com/cmake/community/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


add_executable(test "apps/test.cpp")
target_link_libraries(test wasim-lib)

add_executable(test-tac "apps/test-tac.cpp")
target_link_libraries(test-tac wasim-lib)

# add_executable(independence_check_test "independence_check_test.cpp")
# target_link_libraries(independence_check_test wasim)

# add_executable(solver_test "solver_test.cpp")
# target_link_libraries(solver_test wasim)

# add_executable(symsim_test "symsim_test.cpp")
# target_link_libraries(symsim_test wasim)

# generate dependent header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config/testpath.h.in"  "${CMAKE_CURRENT_SOURCE_DIR}/config/testpath.h")
