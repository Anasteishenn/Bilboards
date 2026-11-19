cmake_minimum_required(VERSION 3.16)

project(AdBoard VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Системные зависимости для SQLite в Linux
find_package(Threads REQUIRED)

# 1. Встраиваемый движок SQLite (компилируется как C)
add_library(sqlite3_lib STATIC
    third_party/sqlite3.c
)

target_include_directories(sqlite3_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(sqlite3_lib PUBLIC
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# 2. Исходный код приложения (автоматически отслеживает новые .cpp файлы)
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(adboard
    ${PROJECT_SOURCES}
)

target_include_directories(adboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(adboard PRIVATE
    sqlite3_lib
)

# 3. Копирование data/schema.sql в папку сборки при каждом билде
add_custom_command(TARGET adboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:adboard>/data
    COMMENT "Copying data folder (schema.sql) to build directory"
)