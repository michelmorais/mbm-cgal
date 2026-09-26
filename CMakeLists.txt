# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 Michel Braz de Morais
cmake_minimum_required(VERSION 3.25.1)
project(mbm_cgal LANGUAGES CXX)
include(CTest)
option(MBM_CGAL_BUILD_PLANAR "Build the planar mesh reduction tool" ON)
if(MBM_CGAL_BUILD_PLANAR)
    add_subdirectory(tools/planar)
endif()
option(MBM_CGAL_BUILD_REMESH "Build the isotropic mesh remeshing tool" ON)
if(MBM_CGAL_BUILD_REMESH)
    add_subdirectory(tools/remesh)
endif()
option(MBM_CGAL_BUILD_AUDIT "Build the read-only mesh audit tool" ON)
if(MBM_CGAL_BUILD_AUDIT)
    add_subdirectory(tools/audit)
endif()
option(MBM_CGAL_BUILD_REPAIR "Build the standalone manifold topology repair tool" ON)
if(MBM_CGAL_BUILD_REPAIR)
    add_subdirectory(tools/repair)
endif()
if(WIN32)
    # vcpkg's MSBuild integration copies this runtime DLL beside the build
    # executable, but CMake's install rules do not know about that copy step.
    # Install only this third-party DLL; Windows system and MSVC runtime DLLs
    # must be provided by the target machine or the Visual C++ Redistributable.
    find_file(MBM_CGAL_GMP_DLL
        NAMES gmp-10.dll
        PATHS
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
            ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES bin
        NO_DEFAULT_PATH)
    if(MBM_CGAL_GMP_DLL)
        install(FILES "${MBM_CGAL_GMP_DLL}" DESTINATION bin)
    endif()
endif()
install(FILES LICENSE NOTICE.md THIRD_PARTY.md DESTINATION share/mbm-cgal)
install(DIRECTORY LICENSES DESTINATION share/mbm-cgal)
