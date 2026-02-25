cmake_minimum_required(VERSION 3.20)

add_library(libodb STATIC
	src/odb_api.c
	src/odb_error.c
	src/odb_handle.c
	src/odb_file.c
	src/odb_file_ops.c
	src/odb_compat.c
	src/odb_db.c
)

set_target_properties(libodb PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)

target_include_directories(libodb
	PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
)

add_executable(odb_cli cli/odb_cli.c)
set_target_properties(odb_cli PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)

target_link_libraries(odb_cli PRIVATE libodb)

enable_testing()
add_executable(odb_tests tests/test_main.c)
set_target_properties(odb_tests PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)

target_link_libraries(odb_tests PRIVATE libodb)
target_compile_definitions(odb_tests PRIVATE ODB_TEST_FIXTURE_DIR=\"${CMAKE_SOURCE_DIR}/databases\")
add_test(NAME odb_tests COMMAND odb_tests)
