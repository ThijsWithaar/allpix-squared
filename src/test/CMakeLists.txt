# SPDX-FileCopyrightText: 2017-2023 CERN and the Allpix Squared authors
# SPDX-License-Identifier: MIT

find_package(Catch2 REQUIRED)
find_package(FMT REQUIRED)

# Include the dependencies
INCLUDE_DIRECTORIES(SYSTEM ${ALLPIX_DEPS_INCLUDE_DIRS})

ADD_EXECUTABLE(
	AllpixTest
	main.cpp
)

TARGET_INCLUDE_DIRECTORIES(AllpixTest PUBLIC SYSTEM ${ALLPIX_DEPS_INCLUDE_DIRS})

# prelink all module libraries
# NOTE: fixes both the RPATH problem as well as the TLS problems
# FIXME: should be removed when we have a better solution
TARGET_LINK_LIBRARIES(AllpixTest PUBLIC
	${ALLPIX_LIBRARIES}
	${_ALLPIX_MODULE_LIBRARIES}
	Catch2::Catch2WithMain
	fmt::fmt
)

include(Catch)
#catch_discover_tests(AllpixTest)
