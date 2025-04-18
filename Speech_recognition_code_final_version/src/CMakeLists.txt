if (WHISPER_SDL2)
    set(TARGET whisper-fuzzy)
    
    file(GLOB SOURCES "./*.cpp")
    add_executable(${TARGET} ${SOURCES})

    include(DefaultTargetOptions)

    target_link_libraries(${TARGET} PRIVATE common common-sdl whisper ${CMAKE_THREAD_LIBS_INIT})

    install(TARGETS ${TARGET} RUNTIME)
endif ()
