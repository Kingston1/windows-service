cmake_minimum_required (VERSION 3.5)

project (ExampleService)

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(ExampleService ${SOURCES} ${HEADERS})