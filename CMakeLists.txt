cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

project(rgr)

add_executable(rgr WIN32
	include/rgr/Core.h
	include/rgr/HandleSet.h
	include/rgr/Application.h
	include/rgr/EventSystem.h
	include/rgr/Window.h
	include/rgr/GraphicsDevice.h
	include/rgr/UI.h
	include/rgr/RegistrationTemplate.h

	src/main.cpp
	src/Application.cpp
	src/Window.cpp
	src/GraphicsDevice.cpp
	src/UI.cpp
	src/RegistrationTemplate.cpp
)

target_include_directories(rgr PRIVATE ./include)
target_link_libraries(rgr PRIVATE gdiplus)