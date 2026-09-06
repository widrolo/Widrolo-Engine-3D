message(STATUS Detected Linux)

target_link_libraries(Afterlife_Engine PRIVATE
        dl
        pthread
)

if (STEAM_ENABLED)
    target_link_libraries(Afterlife_Engine PRIVATE
            ${STEAMWORKS_PATH}/lib/libsteam_api.so
    )

    # valve is at it again
    set_target_properties(Afterlife_Engine PROPERTIES
            BUILD_RPATH "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
    )
endif ()

target_compile_definitions(Afterlife_Engine PRIVATE WE_Linux)
set(CMAKE_CXX_FLAGS_PACKAGING "-O3 -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_PACKAGING "-O3 -DNDEBUG" CACHE STRING "" FORCE)