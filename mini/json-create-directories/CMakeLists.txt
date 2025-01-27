# CMakeList.txt
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("json-create-directories")
find_package(nlohmann_json CONFIG REQUIRED)
find_package(tiny-process-library CONFIG REQUIRED)

add_executable (json-create-directories "json-create-directories.cpp" )
target_link_libraries(json-create-directories PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(json-create-directories PRIVATE tiny-process-library::tiny-process-library)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET json-create-directories PROPERTY CXX_STANDARD 20)
endif()