if(NOT TARGET gwidi_data)
    set(gwidi_data_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_data REQUIRED)
endif()

add_executable(gwidi_data_exec gwidi_gui_data_exec.cc)
target_include_directories(gwidi_data_exec PUBLIC
        ${gwidi_data_INCLUDE_DIRS}
)
target_link_libraries(gwidi_data_exec PRIVATE ${gwidi_data_LIBRARIES})
