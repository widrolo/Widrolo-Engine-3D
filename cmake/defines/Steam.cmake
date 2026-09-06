set(STEAMWORKS_PATH "${CMAKE_SOURCE_DIR}/libs/Steamworks-SDK")

set(STEAM_ENABLED ${OPTION_ENABLED})
set(STEAM_DEBUG_OVERRIDE ${OPTION_DISABLED})

if (STEAM_DEBUG_OVERRIDE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "STEAM_DEBUG_OVERRIDE is enabled, forcing Steam off for Debug builds")
    set(STEAM_ENABLED ${OPTION_DISABLED})
endif ()

# use either the actual app id or 480 if you have none.
set(STEAMAPPID 4188300)

target_compile_definitions(Afterlife_Engine PRIVATE
        STEAM=${STEAM_ENABLED}
        STEAMAPPID=${STEAMAPPID}
)