add_library(linux_send_input)
target_sources(linux_send_input PUBLIC ${CMAKE_CURRENT_LIST_DIR}/LinuxSendInput.cc)
target_include_directories(linux_send_input PUBLIC ${CMAKE_CURRENT_LIST_DIR})

set(linux_send_input_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
set(linux_send_input_LIBRARIES linux_send_input)
