# /*-----------------------------------------------------------------------------------------------------------------------|
# | MIT License (MIT)                                                                                                      |
# | Copyright (C) 2026 by Michel Braz de Morais <michel.braz.morais@gmail.com>                                             |
# |                                                                                                                        |
# | Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated           |
# | documentation files (the "Software"), to deal in the Software without restriction, including without limitation       |
# | the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and       |
# | to permit persons to whom the Software is furnished to do so, subject to the following conditions:                     |
# | The above copyright notice and this permission notice shall be included in all copies or substantial portions.         |
# | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   |
# | WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR  |
# | COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR       |
# | OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.       |
# |-----------------------------------------------------------------------------------------------------------------------*/

cmake_minimum_required(VERSION 3.25.1)
project(mbm_cgal_planar LANGUAGES CXX)
# Intentionally separate from the engine build; CGAL PMP has GPL/commercial terms.
find_package(CGAL 6.0 REQUIRED)
# Do not pass 3.1 here: modern Eigen CMake packages may report a newer major
# version (for example 5.x) and reject an older requested major as incompatible
# even though the package provides the Eigen3::Eigen target.
find_package(Eigen3 REQUIRED NO_MODULE)
add_executable(mbm-cgal-planar main.cpp)
target_compile_features(mbm-cgal-planar PRIVATE cxx_std_17)
target_link_libraries(mbm-cgal-planar PRIVATE CGAL::CGAL Eigen3::Eigen)

install(TARGETS mbm-cgal-planar RUNTIME DESTINATION bin)
if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME planar_geometry COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py $<TARGET_FILE:mbm-cgal-planar>)
    add_test(NAME planar_uv COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test-uv.py $<TARGET_FILE:mbm-cgal-planar>)
endif()
