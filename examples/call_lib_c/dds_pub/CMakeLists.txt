#
# Copyright(c) 2019 to 2022 ZettaScale Technology and others

project(Publisher LANGUAGES C)
cmake_minimum_required(VERSION 3.16)

add_executable(Publisher publisher.c)

target_link_libraries(Publisher RealtimeNetMiddleware)

