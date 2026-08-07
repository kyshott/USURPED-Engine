set(SOURCE_DIR "Source")

set(SOURCES
        action.cpp
        animation.cpp
        assets.cpp
        components.cpp
        entity.cpp
        entitymanager.cpp
        gameengine.cpp
        main.cpp
        physics.cpp
        scene.cpp
        scenemenu.cpp
        sceneplay.cpp
        scenebattle.cpp
        vec2.cpp
        items.cpp
        scenegameover.cpp
)

# Add the executable target
add_executable(A4Student ${SOURCES})
set_target_properties(A4Student PROPERTIES OUTPUT_NAME prog.x)
target_include_directories(A4Student PRIVATE ${PROJECT_SOURCE_DIR}/external/raylib/src)
target_include_directories(A4Student PRIVATE ${PROJECT_SOURCE_DIR}/external/)



target_link_libraries(A4Student imgui)
target_link_libraries(A4Student raylib)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()





