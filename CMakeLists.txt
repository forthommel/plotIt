cmake_minimum_required( VERSION 3.10 )

project(plotIt LANGUAGES CXX)

include(GNUInstallDirs)
include(ExternalProject)

if(EXISTS "$ENV{CMAKE_PREFIX_PATH}/include/boost")
  message(STATUS "Will use $ENV{CMAKE_PREFIX_PATH} as base path for boost")
  set(BOOST_ROOT $ENV{CMAKE_PREFIX_PATH})
  set(Boost_NO_BOOST_CMAKE ON)
endif()

find_package(ROOT REQUIRED COMPONENTS Hist HistPainter Tree)
find_package(Boost REQUIRED COMPONENTS filesystem regex)
find_package(yaml-cpp REQUIRED)

ExternalProject_Add(
  tclap
  URL https://github.com/mirror/tclap/archive/refs/heads/1.2.zip
  BUILD_IN_SOURCE 1
  PATCH_COMMAND ./autotools.sh
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/external
  INSTALL_COMMAND make -ki install
  )

set(SRCS
  src/plotIt.cc
  src/summary.cc
  src/systematics.cc
  src/TH1Plotter.cc
  src/types.cc
  src/utilities.cc
  src/uuid.cc
  )

add_library(plotItlib SHARED ${SRCS})
target_link_libraries(plotItlib PUBLIC Boost::filesystem Boost::regex yaml-cpp)
target_include_directories(plotItlib PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
# workaround, should be inherited from ROOT dependency targets (if present), but is not specified there for versions below 6.18.00
if((${ROOT_VERSION} VERSION_LESS "6.18.00"))
  if(${ROOT_cxx17_FOUND})
    target_compile_features(plotItlib PRIVATE cxx_std_17)
  elseif(${ROOT_cxx14_FOUND})
    target_compile_features(plotItlib PRIVATE cxx_std_14)
  elseif(${ROOT_cxx11_FOUND})
    target_compile_features(plotItlib PRIVATE cxx_std_11)
  endif()
endif()
if(TARGET ROOT::Hist AND TARGET ROOT::Tree AND TARGET ROOT::HistPainter)
  target_link_libraries(plotItlib PUBLIC ROOT::Hist ROOT::HistPainter ROOT::Tree dl)
else()
  target_link_libraries(plotItlib PUBLIC ${ROOT_LIBRARIES} dl)
  target_include_directories(plotItlib PRIVATE ${ROOT_INCLUDE_DIRS})
endif()
add_executable(plotIt src/main.cc)
add_dependencies(plotIt tclap)
target_link_libraries(plotIt PUBLIC plotItlib)
target_include_directories(plotIt PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> ${CMAKE_CURRENT_BINARY_DIR}/external/include)
install(TARGETS plotIt
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
install(TARGETS plotItlib DESTINATION ${CMAKE_INSTALL_LIBDIR})
file(GLOB plotIt_INCLUDES include/*.h)
install(FILES ${plotIt_INCLUDES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/plotIt)
