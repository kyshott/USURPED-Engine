#imgui library
set(IMGUISRC
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    rlImGui.cpp
    imgui_stdLib.cpp
    imgui.cpp)
add_library(imgui STATIC ${IMGUISRC})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(imgui PRIVATE ${PROJECT_SOURCE_DIR}/external/raylib/src)
set_target_properties(imgui PROPERTIES OUTPUT_NAME imgui)
set_target_properties(imgui PROPERTIES FOLDER "Dependencies")

