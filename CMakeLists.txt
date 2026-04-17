cmake_minimum_required(VERSION 3.20)
project(Game_Project CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# --- OGL3D static library ---
add_library(OGL3D STATIC
    OGL3D/source/OGL3D/Game/Game.cpp
    OGL3D/source/OGL3D/Graphics/GraphicsEngine.cpp
    OGL3D/source/OGL3D/Graphics/ShaderProgram.cpp
    OGL3D/source/OGL3D/Graphics/UniformBuffer.cpp
    OGL3D/source/OGL3D/Graphics/VertexArrayObject.cpp
    OGL3D/source/OGL3D/Window/GWindow.cpp
    OGL3D/vendor/glad/src/glad.c
    OGL3D/vendor/glad/src/glad_wgl.c
)

target_include_directories(OGL3D PUBLIC
    OGL3D/include
    OGL3D/source
    OGL3D/vendor/glad/include
    OGL3D/vendor/glad/src
    OGL3D/vendor/stb_img
)

target_link_libraries(OGL3D PUBLIC OpenGL::GL)

# --- Game executable ---
add_executable(Game_Project Game/main.cpp)

target_link_libraries(Game_Project PRIVATE OGL3D)

# Copy assets next to the executable after build
add_custom_command(TARGET Game_Project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Game_Project>/Assets"
)
