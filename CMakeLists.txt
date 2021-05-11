cmake_minimum_required(VERSION 3.15)

set(SDKTOOLS_VERSION 1.0.0)
project(SDKTOOLS
	VERSION ${SDKTOOLS_VERSION}
	DESCRIPTION "A simple utf-8 based unicode decoding and encoding library"
	HOMEPAGE_URL "https://github.com/northwindtraders/beicode"
	LANGUAGES CXX)

add_executable(SDKTOOLS 	Source.cpp)
target_include_directories(SDKTOOLS PRIVATE wu_camera-sdk/inc)
target_link_directories(SDKTOOLS PRIVATE wu_camera-sdk/lib)
target_compile_features(SDKTOOLS PRIVATE cxx_std_17)