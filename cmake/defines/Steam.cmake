set(STEAMWORKS_PATH "${CMAKE_SOURCE_DIR}/libs/Steamworks-SDK")

set(STEAM_ENABLED ${OPTION_ENABLED})

target_compile_definitions(Afterlife_Engine PRIVATE STEAM=${STEAM_ENABLED})