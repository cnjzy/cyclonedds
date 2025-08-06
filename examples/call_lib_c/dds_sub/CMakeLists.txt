project(Subscriber LANGUAGES C)
cmake_minimum_required(VERSION 3.16)

add_executable(Subscriber subscriber.c)

# Both executables need to be linked to the idl data type library and
target_link_libraries(Subscriber RealtimeNetMiddleware)
