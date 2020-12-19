cmake_minimum_required(VERSION 3.16)
project(chit-pis)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCES
        colors.h
        EditableText.cpp
        EditableText.h
        LineEdit.cpp
        LineEdit.h
        Lines.cpp
        Lines.h
        main.cpp
        MessagePopup.cpp
        MessagePopup.h
        Popup.cpp
        Popup.h
        Screen.cpp
        Screen.h
        StatusScreen.cpp
        StatusScreen.h
        utils.cpp
        utils.h
)

add_executable(chit-pis ${SOURCES})
