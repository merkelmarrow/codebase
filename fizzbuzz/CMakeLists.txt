# CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

project ("fizzbuzz")

add_executable (fizzbuzz "fizzbuzz.cpp" )

target_compile_features(fizzbuzz PRIVATE cxx_std_17)