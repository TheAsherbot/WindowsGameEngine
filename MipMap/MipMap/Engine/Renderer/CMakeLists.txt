# Enigne\Renderer

add_subdirectory(ConsoleRenderer)
add_subdirectory(DirectX)

file(GLOB CORE_SRC CONFIGURE_DEPENDS *.cpp *.h)

target_sources(MipMap PRIVATE ${CORE_SRC})