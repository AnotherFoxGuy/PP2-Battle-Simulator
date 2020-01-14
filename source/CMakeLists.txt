project(PP2)


####################################################################################################
#  SOURCE FILES
####################################################################################################

set(SOURCE_FILES
        ThreadPool.h
        explosion.{h,cpp}
        game.{h,cpp}
        particle_beam.{h,cpp}
        rocket.{h,cpp}
        smoke.{h,cpp}
        Algorithms.{h,cpp}
        tank.{h,cpp}
        template.{h,cpp}
        defines.h
        Grid.{h,cpp})

include(SourceFileUtils)

# Expand file extensions (i.e. path/to/file.{h,cpp} becomes path/to/file.h;path/to/file.cpp)
expand_file_extensions(SOURCE_FILES ${SOURCE_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC .)
if (USE_PACKAGE_MANAGER)
    # Generate source groups for use in IDEs
    generate_source_groups(${SOURCE_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::sdl2 CONAN_PKG::sdl2_image CONAN_PKG::sdl2_ttf CONAN_PKG::tbb)
else ()
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb ${SDL2_LIBRARIES} ${SDL2_image_LIBRARIES} ${SDL2_ttf_LIBRARIES})
endif ()

if (USE_PACKAGE_MANAGER)
    include(cotire)
    cotire(${PROJECT_NAME})
endif ()


if (MSVC)
    # set startup project for Visual Studio Builds
    set_property(
            DIRECTORY ${PROJECT_SOURCE_DIR}
            PROPERTY
            VS_STARTUP_PROJECT ${PROJECT_NAME}
    )
    set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif ()

if (ENABLE_EASY_PROFILER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USING_EASY_PROFILER)
    target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::easy_profiler)
endif ()

add_custom_target(
        copy_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/ ${CMAKE_BINARY_DIR}/assets/
        COMMENT "Copy the resources to the compiled directory"
        VERBATIM
)

