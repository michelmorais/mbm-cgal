# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 Michel Braz de Morais
cmake_minimum_required(VERSION 3.25.1)
project(mbm_cgal LANGUAGES CXX)
include(CTest)
option(MBM_CGAL_BUILD_PLANAR "Build the planar mesh reduction tool" ON)
if(MBM_CGAL_BUILD_PLANAR)
    add_subdirectory(tools/planar)
endif()
install(FILES LICENSE NOTICE.md THIRD_PARTY.md DESTINATION share/mbm-cgal)
install(DIRECTORY LICENSES DESTINATION share/mbm-cgal)
