find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 IMPORTED_TARGET sdl2)
pkg_check_modules(OPENGL IMPORTED_TARGET opengl)

add_subdirectory(common)

if(SDL2_FOUND)
    add_subdirectory(sdl2)
endif()
