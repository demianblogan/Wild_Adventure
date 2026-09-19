file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*Tests.cpp")

add_executable(PlatformerTests TestMain.cpp ${TEST_SOURCES})

target_include_directories(PlatformerTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(PlatformerTests PRIVATE PlatformerLib)

if (MSVC)
	# C5285 fires inside the vendored doctest.h itself (a std::tuple
	# specialization it uses internally) -- not something to patch in
	# third-party code, so it is disabled just for this target.
	target_compile_options(PlatformerTests PRIVATE /W4 /WX /wd5285)
else()
	target_compile_options(PlatformerTests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_command(TARGET PlatformerTests POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_RUNTIME_DLLS:PlatformerTests>" "$<TARGET_FILE_DIR:PlatformerTests>"
	COMMAND_EXPAND_LISTS
)

add_test(NAME PlatformerTests COMMAND PlatformerTests)
