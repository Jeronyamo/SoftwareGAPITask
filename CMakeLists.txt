cmake_minimum_required(VERSION 3.8)
project(gapi_test)

option(USE_OPENGL "Use reference OpenGL implementation" ON)

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 17)

if(USE_OPENGL)
  add_definitions(-DUSE_OPENGL)
endif()

include_directories(".")
if(WIN32)
  link_directories(${ADDITIONAL_LIBRARY_DIRS})
endif()

find_package(OpenMP)

#uncomment this to detect broken memory problems via gcc sanitizers
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

if(USE_OPENGL)
  set(FREEGLUT_BUILD_STATIC_LIBS ON)
  set(FREEGLUT_BUILD_DEMOS OFF)
  add_subdirectory("./freeglut")
  include_directories("freeglut/include")

  add_executable(testapp main.cpp 
                         Bitmap.cpp
                         lgapi_ref.cpp
                         scene.cpp)
else()
  add_executable(testapp main.cpp 
                         Bitmap.cpp
                         lgapi.cpp
                         scene.cpp)
endif()

set_target_properties(testapp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

if(USE_OPENGL)
  target_link_libraries(testapp LINK_PUBLIC OpenMP::OpenMP_CXX freeglut_static) # for reference implementation
else()
  target_link_libraries(testapp LINK_PUBLIC OpenMP::OpenMP_CXX)                 # for your implementation
endif()
