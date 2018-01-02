cmake_minimum_required(VERSION 2.8)

project(myext)

set(SOURCE_HEADERS
        minicsv.h
		dateparser.h)

set(SOURCE_FILES
		dateparser.cpp)


# make static library called libmyxrm
# that we are going to link to
# in the root CMakeLists.txt file
add_library(myext
		STATIC
		${SOURCE_FILES})
