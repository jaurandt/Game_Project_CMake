cmake_minimum_required(VERSION 3.20)
project(QuantumBoxExplorer VERSION 1.0.0 LANGUAGES C CXX) # Version the shipped application.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC) # Ensure startup errors and test failures unwind safely on Windows.
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/EHsc>) # Enable standard C++ exception semantics for MSVC.
endif() # Finish compiler-specific exception support.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # Bundle the MSVC runtime statically for portable ZIPs.
include(CTest) # Enable physics verification through CTest.
add_library(QuantumPhysics INTERFACE) # Expose physics without Win32 or OpenGL dependencies.
target_include_directories(QuantumPhysics INTERFACE OGL3D/include) # Share the model with tests and rendering.
if(BUILD_TESTING) # Include physics checks in developer and CI builds.
    add_executable(PhysicsTests tests/PhysicsTests.cpp) # Build independent numerical and sampling tests.
    target_link_libraries(PhysicsTests PRIVATE QuantumPhysics) # Use the exact model consumed by the application.
    foreach(check boundaries normalization interference energy sampling validation) # Report each invariant separately.
        add_test(NAME physics.${check} COMMAND PhysicsTests ${check}) # Run checks even in Release where assertions are disabled.
    endforeach() # Finish registering physics checks.
endif() # Finish optional test setup.
option(BUILD_EXPLORER "Build the Windows visualization" ON) # Allow physics checks on machines without graphics.
if(BUILD_EXPLORER) # Build and package the native visualization when requested.
if(NOT WIN32) # Explain the platform requirement before resolving graphics dependencies.
    message(FATAL_ERROR "Quantum Box Explorer requires Windows; use -DBUILD_EXPLORER=OFF for physics tests") # Support portable model testing.
endif() # Finish the platform check.

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

target_link_libraries(Game_Project PRIVATE OGL3D QuantumPhysics) # Connect the renderer to the tested model.
set_target_properties(Game_Project PROPERTIES OUTPUT_NAME QuantumBoxExplorer) # Give the downloadable binary its product name.

# Copy assets next to the executable after build
add_custom_command(TARGET Game_Project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Game_Project>/Assets"
)
install(TARGETS Game_Project RUNTIME DESTINATION .) # Place the executable at the ZIP root.
install(DIRECTORY Assets DESTINATION .) # Ship shaders beside the executable.
install(FILES README.md RELEASE_NOTES.md THIRD_PARTY_NOTICES.md DESTINATION .) # Include usage and dependency attribution.
set(CPACK_GENERATOR ZIP) # Produce a portable downloadable archive.
set(CPACK_PACKAGE_NAME QuantumBoxExplorer) # Identify the product in package metadata.
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}) # Keep archive and project versions aligned.
set(CPACK_PACKAGE_FILE_NAME "QuantumBoxExplorer-${PROJECT_VERSION}-windows-x64") # Use a predictable release asset name.
include(CPack) # Enable the package target and CPack configuration.
endif() # Finish the Windows application configuration.
