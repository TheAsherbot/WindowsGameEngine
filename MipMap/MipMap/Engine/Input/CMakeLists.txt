# Enigne\Input

add_subdirectory(ConsoleInput)

file(GLOB CORE_SRC CONFIGURE_DEPENDS *.cpp *.h)

target_sources(MipMap PRIVATE ${CORE_SRC})