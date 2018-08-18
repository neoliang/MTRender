cmake_minimum_required( VERSION 2.6 )
project( MTRender )

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_subdirectory(rapidjson)
include_directories( External/Include )
include_directories( Common/Include )
include_directories( glm )
include_directories(rapidjson/include)
find_library( OPENGLES3_LIBRARY GLESv2 "OpenGL ES v3.0 library")
find_library( EGL_LIBRARY EGL "EGL 1.4 library" )
SUBDIRS( Common
         Hello_Triangle
		 DemoCreateResReturnIM
	     DemoCreateResReturnDelay		 
		)	
		