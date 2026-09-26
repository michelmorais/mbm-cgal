cmake_minimum_required(VERSION 3.25.1)
project(mbm_cgal_remesh LANGUAGES CXX)
# Keep CGAL in the optional offline worker, separate from the engine runtime.
find_package(CGAL 6.0 REQUIRED)
# Do not pass 3.1 here: modern Eigen CMake packages may report a newer major
# version (for example 5.x) and reject an older requested major as incompatible
# even though the package provides the Eigen3::Eigen target.
find_package(Eigen3 REQUIRED NO_MODULE)
add_executable(mbm-cgal-remesh main.cpp)
target_compile_features(mbm-cgal-remesh PRIVATE cxx_std_17)
target_link_libraries(mbm-cgal-remesh PRIVATE CGAL::CGAL Eigen3::Eigen)

install(TARGETS mbm-cgal-remesh RUNTIME DESTINATION bin)
if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME isotropic_remesh COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py $<TARGET_FILE:mbm-cgal-remesh>)
endif()
