# root

add_executable (MipMap MipMap.cpp MipMap.h)

# Link the Engine library to your executable
target_include_directories(MipMap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Enter subdirectories
add_subdirectory(Engine)
add_subdirectory(Games)

set_property(TARGET MipMap PROPERTY CXX_STANDARD 20)

target_link_libraries(MipMap PRIVATE
    d3d12
    dxgi
    D3DCompiler
    windowscodecs
)