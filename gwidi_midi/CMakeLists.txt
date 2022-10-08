cmake_minimum_required(VERSION 3.20)
project(gwidi_midi)
set(version 1.0)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET gwidi_midi)
    set(gwidi_midi_DIR ${CMAKE_CURRENT_LIST_DIR})
    find_package(gwidi_midi REQUIRED)
endif()

option(BUILD_TESTS "Build test exec" OFF)

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(test)
endif()

# Define installation for outside projects
#install(
#        FILES
#        include/gwidi_midi_parser.h
#        midifile/include/Binasc.h
#        midifile/include/MidiEvent.h
#        midifile/include/MidiEventList.h
#        midifile/include/MidiFile.h
#        midifile/include/MidiMessage.h
#        midifile/include/Options.h
#        DESTINATION
#        include/gwidi_midi-${version}
#)
#
#install(
#        TARGETS gwidi_midi
#        DESTINATION lib/gwidi_midi-${version}
#        EXPORT gwidi_midi-targets
#)
#
#install(
#        TARGETS midifile
#        DESTINATION lib/midifile
#        EXPORT midifile-targets
#)
#
#
#install(
#        EXPORT
#        gwidi_midi-targets
#        DESTINATION lib/gwidi_midi-${version}
#)
#
#install(
#        EXPORT
#        midifile-targets
#        DESTINATION lib/midifile
#)
