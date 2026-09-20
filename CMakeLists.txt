cmake_minimum_required(VERSION 3.25)

project(Platformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SFML 3.1.0, built from source into libs/SFML (see libs/SFML/README.md):
# vcpkg's own sfml port has not been updated past 3.0.2 yet.
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/libs/SFML")
find_package(SFML 3.1 COMPONENTS Graphics Window System Audio Main CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# The project is feature-complete, so new source files are not expected; a
# recursive glob is far easier to keep correct than a hand-maintained list of
# ~200 paths. CONFIGURE_DEPENDS still re-runs it on the next configure if a
# file is ever added or removed.
file(GLOB_RECURSE PLATFORMER_LIB_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
list(REMOVE_ITEM PLATFORMER_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(SORT PLATFORMER_LIB_SOURCES)

# DualSense/DualShock rumble + lightbar support: Ohjurot/DualSense-Windows
# (MIT), vendored in full under libs/DualSenseWindows/, since Windows has no
# public vendor API for these controllers (XInput doesn't cover them at
# all) -- it talks to the hardware directly over raw HID instead. See
# libs/DualSenseWindows/LICENSE.
file(GLOB DUALSENSE_WINDOWS_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/libs/DualSenseWindows/src/DualSenseWindows/*.cpp"
)

# Everything except main() lives in a library, so the test executable below
# can link the whole game without also pulling in a competing main().
add_library(PlatformerLib STATIC ${PLATFORMER_LIB_SOURCES} ${DUALSENSE_WINDOWS_SOURCES})

# Third-party code: builds at /W3 without warnings-as-errors, unlike the
# rest of this target below.
set_source_files_properties(${DUALSENSE_WINDOWS_SOURCES} PROPERTIES
	COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/W3;/WX->"
)

target_link_libraries(PlatformerLib PUBLIC
	SFML::Graphics
	SFML::Window
	SFML::System
	SFML::Audio
	nlohmann_json::nlohmann_json
	Xinput.lib
	Setupapi.lib
	Hid.lib
)

# DS5W_BUILD_LIB: DualSenseWindows is vendored as source, not a DLL, so its
# API must not be declared __declspec(dllimport) (its default with neither
# macro defined). PUBLIC so it stays consistent for every translation unit
# that includes its headers, including the test executable below.
target_compile_definitions(PlatformerLib PUBLIC DS5W_BUILD_LIB)

target_include_directories(PlatformerLib PUBLIC src
	"${CMAKE_CURRENT_SOURCE_DIR}/libs/DualSenseWindows/include"
	# DualSenseWindows's own .cpp files reach for a few headers (DS_CRC32.h,
	# DS5_Input.h, DS5_Output.h) that live next to them under src/ rather
	# than in the public include/ folder above, via the same
	# <DualSenseWindows/...> angle-bracket form -- so src/ needs to be on
	# the include path too, matching how the vendored library is set up
	# upstream.
	"${CMAKE_CURRENT_SOURCE_DIR}/libs/DualSenseWindows/src")

if (MSVC)
	# C4458: a constructor/setter parameter deliberately shares a name with the
	# member it initializes (e.g. `SetColor(sf::Color color) { this->color =
	# color; }`). That is an idiomatic, safe pattern used throughout this
	# codebase, not a real hazard, so the warning is noise.
	target_compile_options(PlatformerLib PRIVATE /W4 /WX /wd4458)
else()
	target_compile_options(PlatformerLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# WIN32: builds the shipped game as a GUI app with no console window
# attached (a plain add_executable defaults to the console subsystem on
# Windows). SFML::Main supplies the WinMain a WIN32 subsystem app needs and
# forwards straight to this project's own int main() in main.cpp, so nothing
# about that file has to change.
add_executable(Platformer WIN32 src/main.cpp)
target_link_libraries(Platformer PRIVATE PlatformerLib SFML::Main)

# assets/ and data/ are loaded with paths relative to the current working
# directory. Some launch paths (Visual Studio's CMake integration among them)
# don't honour VS_DEBUGGER_WORKING_DIRECTORY below and run the executable
# with its own output directory as the working directory instead of the
# project root, which would otherwise leave it unable to find either folder.
# Mirroring both next to the built executable makes it work either way,
# exactly like the shipped release build (exe sitting next to assets/data).
set_target_properties(Platformer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_custom_command(TARGET Platformer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		"${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:Platformer>/assets"
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		"${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:Platformer>/data"
)

# SFML is vendored as shared libraries (see libs/SFML/README.md); copy the
# DLLs it and its own dependencies need next to every executable that links
# it, matching what vcpkg's toolchain used to do automatically.
add_custom_command(TARGET Platformer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_RUNTIME_DLLS:Platformer>" "$<TARGET_FILE_DIR:Platformer>"
	COMMAND_EXPAND_LISTS
)

enable_testing()
add_subdirectory(tests)
