if(NOT TARGET gwidi_server_client)
    set(gwidi_server_client_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_server_client REQUIRED)
endif()

add_executable(gwidi_server_client_exec main.cc)
target_include_directories(gwidi_server_client_exec PUBLIC
        ${gwidi_server_client_INCLUDE_DIRS}
)
target_link_libraries(gwidi_server_client_exec PUBLIC ${gwidi_server_client_LIBRARIES})
