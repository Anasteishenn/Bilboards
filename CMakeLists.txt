cmake_minimum_required(VERSION 3.16)

project(AdboardApp VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(sqlite3_lib STATIC
    third_party/sqlite3.c
)

target_include_directories(sqlite3_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(sqlite3_lib PRIVATE
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

file(GLOB_RECURSE PROJECT_SOURCES 
    "src/*.cpp"
)

add_executable(adboard_app 
    ${PROJECT_SOURCES}
)

target_include_directories(adboard_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(adboard_app PRIVATE
    sqlite3_lib
    Threads::Threads
)

add_custom_command(TARGET adboard_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:adboard_app>/data
    COMMENT "Copying data folder (schema.sql) to build directory"
)