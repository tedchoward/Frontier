cmake_minimum_required(VERSION 3.20)

add_library(libodb STATIC
	src/odb_api.c
	src/odb_error.c
	src/odb_handle.c
	src/odb_file.c
	src/odb_file_ops.c
	src/odb_compat.c
	src/odb_db.c
	src/legacy/odb_legacy.c
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
target_include_directories(odb_cli PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

add_executable(odb_dump tools/odb_dump.c)
set_target_properties(odb_dump PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)
target_link_libraries(odb_dump PRIVATE libodb)
target_include_directories(odb_dump PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

add_executable(odb_packinfo tools/odb_packinfo.c)
set_target_properties(odb_packinfo PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)
target_link_libraries(odb_packinfo PRIVATE libodb)
target_include_directories(odb_packinfo PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

enable_testing()
add_executable(odb_tests tests/test_main.c)
set_target_properties(odb_tests PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)

target_link_libraries(odb_tests PRIVATE libodb)
target_compile_definitions(odb_tests PRIVATE ODB_TEST_FIXTURE_DIR=\"${CMAKE_SOURCE_DIR}/databases\")
target_include_directories(odb_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
add_test(NAME odb_tests COMMAND odb_tests)

add_executable(odb_internal_tests tests/test_internal.c)
set_target_properties(odb_internal_tests PROPERTIES
	C_STANDARD 23
	C_STANDARD_REQUIRED YES
)
target_link_libraries(odb_internal_tests PRIVATE libodb)
target_include_directories(odb_internal_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
add_test(NAME odb_internal_tests COMMAND odb_internal_tests)
