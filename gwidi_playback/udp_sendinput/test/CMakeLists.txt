if(NOT TARGET udp_sendinput)
    set(udp_sendinput_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(udp_sendinput REQUIRED)
endif()

add_executable(udp_sendinput_exec main.cc)
target_include_directories(udp_sendinput_exec PUBLIC
        ${udp_sendinput_INCLUDE_DIRS}
)
target_link_libraries(udp_sendinput_exec PRIVATE ${udp_sendinput_LIBRARIES})
