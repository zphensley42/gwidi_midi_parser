if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

if(NOT TARGET gwidi_server_client)
    set(gwidi_server_client_DIR ${CMAKE_CURRENT_LIST_DIR}/../../gwidi_server_client)
    find_package(gwidi_server_client REQUIRED)
endif()

# Define targets
add_library(udp_sendinput)
target_sources(udp_sendinput PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/UdpSendInput.cc
)

target_include_directories(udp_sendinput PUBLIC
        ${gwidi_server_client_INCLUDE_DIRS}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(udp_sendinput PUBLIC
        ${gwidi_server_client_LIBRARIES}
        spdlog::spdlog
)

set(udp_sendinput_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
set(udp_sendinput_LIBRARIES udp_sendinput)
