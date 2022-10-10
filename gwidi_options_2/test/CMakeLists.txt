if(NOT TARGET gwidi_options_2)
    set(gwidi_options_2_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_options_2 REQUIRED)
endif()

add_executable(gwidi_options_2_exec gwidi_options_2_exec.cc)
target_include_directories(gwidi_options_2_exec PUBLIC
        ${gwidi_options_2_INCLUDE_DIRS}
)
target_link_libraries(gwidi_options_2_exec PRIVATE ${gwidi_options_2_LIBRARIES})
