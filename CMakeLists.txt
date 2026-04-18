cmake_minimum_required(VERSION 3.28)
project(sfml-archer-duel LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
)

set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_INSTALL_LIBRARIES ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(sfml)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Button.cpp
    src/Collision.cpp
    src/Entity.cpp
    src/FileIO.cpp
    src/GameManager.cpp
    src/Input.cpp
    src/Powerup.cpp
    src/Render.cpp
    src/AIEnemy.cpp
    src/Arrow.cpp
    src/Destructible.cpp
    src/Player.cpp
    src/ArenaState.cpp
    src/BuyingMenuState.cpp
    src/CharacterCreationState.cpp
    src/MainMenuState.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/saves"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/saves"
)
