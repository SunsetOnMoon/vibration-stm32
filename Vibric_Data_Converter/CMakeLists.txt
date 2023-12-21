cmake_minimum_required(VERSION 3.26)
project(Vibric_Data_Converter C)

set(CMAKE_C_STANDARD 11)

add_executable(Vibric_Data_Converter main.c
        vibric/vibric.h
        vibric/vibric.c
)
