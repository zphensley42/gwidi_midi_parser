if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

include(FetchContent)

FetchContent_Declare(inipp
        GIT_REPOSITORY https://github.com/zphensley42/inipp.git
        GIT_TAG        develop
)
FetchContent_MakeAvailable(inipp)


set(OPTIONS_HDRS ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(gwidi_options)
target_sources(gwidi_options PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gwidi_options_parser.cc
)
target_include_directories(gwidi_options PUBLIC
        ${OPTIONS_HDRS}
)
target_link_libraries(gwidi_options PUBLIC spdlog::spdlog inipp::inipp)


set(gwidi_options_INCLUDE_DIRS ${OPTIONS_HDRS})
set(gwidi_options_LIBRARIES gwidi_options)
