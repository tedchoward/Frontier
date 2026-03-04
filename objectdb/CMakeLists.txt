cmake_minimum_required(VERSION 3.20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(objectdb
	src/main.c
	src/app.c
)

set_target_properties(objectdb PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)

target_include_directories(objectdb PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src
	${GTK3_INCLUDE_DIRS}
)

target_compile_options(objectdb PRIVATE ${GTK3_CFLAGS_OTHER})

target_link_directories(objectdb PRIVATE ${GTK3_LIBRARY_DIRS})

target_link_libraries(objectdb PRIVATE
	libodb
	${GTK3_LIBRARIES}
)
