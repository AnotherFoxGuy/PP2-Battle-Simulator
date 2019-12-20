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
        surface.{h,cpp}
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

    target_include_directories(${PROJECT_NAME} PUBLIC CONAN_PKG::taskflow)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL CONAN_PKG::sdl2 CONAN_PKG::freeimage CONAN_PKG::taskflow)
    #target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)

else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL SDL2::SDL2)

    target_include_directories(${PROJECT_NAME} PRIVATE ${FreeImage_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FREEIMAGE_LIBRARY})
endif ()

if (USE_PACKAGE_MANAGER)
    include(cotire)
    cotire(${PROJECT_NAME})
endif ()


if (MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE "ws2_32.lib")
    # set startup project for Visual Studio Builds
    set_property(
            DIRECTORY ${PROJECT_SOURCE_DIR}
            PROPERTY
            VS_STARTUP_PROJECT ${PROJECT_NAME}
    )
    # set Visual Studio working directory to the Configuration directory
    set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/source/$(Configuration)"
    )
endif ()

if (ENABLE_EASY_PROFILER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USING_EASY_PROFILER)
    target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::easy_profiler)
endif ()

add_custom_target(
        copy_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/
        COMMENT "Copy the resources to the compiled directory"
        VERBATIM
)

