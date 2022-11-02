if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

if(NOT TARGET strutil)
    set(strutil_DIR ${CMAKE_CURRENT_LIST_DIR}/../strutil/)
    find_package(strutil REQUIRED)
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)


set(OPTIONS_HDRS ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(gwidi_options_2)
target_sources(gwidi_options_2 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gwidi_options_2_parser.cc
)
target_include_directories(gwidi_options_2 PUBLIC
        ${OPTIONS_HDRS}
)
target_link_libraries(gwidi_options_2 PUBLIC spdlog::spdlog strutil nlohmann_json::nlohmann_json)
#target_link_libraries(gwidi_options_2 PRIVATE nlohmann_json::nlohmann_json)


set(gwidi_options_2_INCLUDE_DIRS ${OPTIONS_HDRS})
set(gwidi_options_2_LIBRARIES gwidi_options_2 nlohmann_json strutil)
