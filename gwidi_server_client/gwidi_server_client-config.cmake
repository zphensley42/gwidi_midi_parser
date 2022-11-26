if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

# Define targets
add_library(gwidi_server_client)
target_sources(gwidi_server_client PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/GwidiServerClient.cc
)

target_include_directories(gwidi_server_client PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(gwidi_server_client PUBLIC
        spdlog::spdlog
)

set(gwidi_server_client_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
set(gwidi_server_client_LIBRARIES gwidi_server_client)
