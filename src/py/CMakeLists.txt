# SPDX-FileCopyrightText: 2017-2023 CERN and the Allpix Squared authors
# SPDX-License-Identifier: MIT

OPTION(BUILD_PY "Build python bindings" ON)
IF(BUILD_PY)
    FIND_PACKAGE(pybind11 CONFIG REQUIRED)

    # create module and link the libs
    pybind11_add_module(pyallpix allpix.cpp)

    TARGET_INCLUDE_DIRECTORIES(pyallpix PUBLIC SYSTEM ${ALLPIX_DEPS_INCLUDE_DIRS})

    # prelink all module libraries
    # NOTE: fixes both the RPATH problem as well as the TLS problems
    # FIXME: should be removed when we have a better solution
    TARGET_LINK_LIBRARIES(pyallpix PUBLIC ${ALLPIX_LIBRARIES} ${_ALLPIX_MODULE_LIBRARIES})

    # set install location
    INSTALL(
        TARGETS pyallpix
        DESTINATION "${Python3_SITELIB}/${PROJECT_NAME}"
    )
ENDIF()
