# CMakeList.txt : CMake project for Ex1_01.cpp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.28)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED)
set (CMAKE_CXX_SCAN_FOR_MODULES ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Ex1_01.cpp")

# Add source to this project's executable.
add_executable (Ex1_01.cpp "Ex1_01.cpp.cpp" )

# TODO: Add tests and install targets if needed.
