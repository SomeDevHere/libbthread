# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.")
endif()

# Set minimum cmake version + cmake settings
cmake_minimum_required (VERSION 3.9)

# Specify the project, and set the use of c++ and c
project(cancelthread LANGUAGES CXX C)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_EXAMPLES "Build examples using library" ON)

add_compile_options(-ffunction-sections -fdata-sections -Wall -Wextra -Wpedantic)
set(SRCS pt-cancel.c pt-docancel.c pt-init.c pt-internal.c pt-setcancelstate.c pt-setcanceltype.c pt-testcancel.c)
if(BUILD_SHARED_LIBS)
add_library(cancelthread SHARED ${SRCS})
else()
add_library(cancelthread STATIC ${SRCS})
endif()
target_include_directories(cancelthread PUBLIC ${PROJECT_SOURCE_DIR})
set_property(TARGET cancelthread PROPERTY C_STANDARD 11)

find_library(LIBPTHREAD pthread)
if (LIBPTHREAD) 
	target_link_libraries(cancelthread -lz -ldl -lm -lpthread)
else()
	target_link_libraries(cancelthread -lz -ldl -lm)
endif()

if(BUILD_EXAMPLES)
add_executable(bthread_test pt-test.c)
target_link_libraries(bthread_test PRIVATE cancelthread)
endif()

install(FILES bthread.h DESTINATION include)
install(TARGETS cancelthread 
      RUNTIME
        DESTINATION bin
      LIBRARY
        DESTINATION lib
      ARCHIVE
        DESTINATION lib)
