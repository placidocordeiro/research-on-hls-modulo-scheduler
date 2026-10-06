cmake_minimum_required(VERSION 3.14)
project(hls_modulo_scheduler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build tipo Release por padrao
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Flags comuns
add_compile_options(-Wall -Wextra)

# nlohmann/json via submodule
include_directories(${CMAKE_SOURCE_DIR}/third_party/json/single_include)

# Binarios vao para a raiz do build/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# --- ms-mii: calculo de MII ---
add_executable(ms-mii
    sched/main_mii.cpp
    sched/instance.cpp
    sched/mii.cpp
)

# --- smoke test: roda ms-mii nas instancias prontas ---
add_custom_target(test-mii
    DEPENDS ms-mii
    COMMAND ./ms-mii ${CMAKE_SOURCE_DIR}/instances/simple_add_chain.json
    COMMAND ./ms-mii ${CMAKE_SOURCE_DIR}/instances/mac_loop.json
    COMMAND ./ms-mii ${CMAKE_SOURCE_DIR}/instances/loop_with_shift.json
    COMMAND ./ms-mii ${CMAKE_SOURCE_DIR}/instances/sha_transform.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Smoke test: ms-mii nas instancias prontas"
)
