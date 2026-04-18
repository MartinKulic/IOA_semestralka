cmake_minimum_required(VERSION 4.1)
project(IOA_semestralka)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(external/ftxui)
include_directories(external/ftxui/include)

add_executable(IOA_semestralka
        main.cpp
        fStar/fStar.cpp
        fStar/fStar.hpp
        gui/gui.cpp
        gui/gui.hpp
        fStar/Alg.cpp
        fStar/Alg.hpp)

#add_executable(main main.cpp)
target_link_libraries(IOA_semestralka
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)



