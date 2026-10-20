# Enigne\

add_subdirectory(Renderer)
add_subdirectory(Input)

file(GLOB CORE_SRC CONFIGURE_DEPENDS *.cpp *.h)

target_sources(MipMap PRIVATE ${CORE_SRC})