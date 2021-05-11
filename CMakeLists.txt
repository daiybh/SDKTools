cmake_minimum_required(VERSION 3.15)

set(SDKTOOLS_VERSION 1.0.0)
include(ExternalProject)
ExternalProject_Add(SDK
  GIT_REPOSITORY    git@github.com:daiybh/wu_camera-sdk.git  
)

project(SDKTOOLS
	VERSION ${SDKTOOLS_VERSION}
	DESCRIPTION "A simple utf-8 based unicode decoding and encoding library"
	HOMEPAGE_URL "https://github.com/northwindtraders/beicode"
	LANGUAGES CXX)

