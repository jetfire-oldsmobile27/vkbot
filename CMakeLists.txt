cmake_minimum_required(VERSION 3.17)
project(VKBOT VERSION 0.1.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_EXAMPLES "Build examples" ON)


find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)


file(GLOB_RECURSE VKBOT_SOURCES "src/*.cpp")
file(GLOB_RECURSE VKBOT_HEADERS "include/*.hpp")

add_library(vkbot STATIC ${VKBOT_SOURCES} ${VKBOT_HEADERS})

target_include_directories(vkbot
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vkbot>
)

if(TARGET boost::boost)
    set(_BOOST_TARGET boost::boost)
else()
    set(_BOOST_TARGET Boost::system)
endif()

if(TARGET openssl::openssl)
    set(_OPENSSL_TARGET openssl::openssl)
else()
    set(_OPENSSL_TARGET OpenSSL::SSL OpenSSL::Crypto)
endif()

target_link_libraries(vkbot
    PUBLIC
        ${_BOOST_TARGET}
        ${_OPENSSL_TARGET}
        nlohmann_json::nlohmann_json
)

# ---------- install ----------
install(TARGETS vkbot EXPORT VKBOTTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES ${VKBOT_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vkbot
)

configure_package_config_file(
    cmake/VKBOTConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/VKBOTConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VKBOT
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/VKBOTConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/VKBOTConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VKBOTConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VKBOT
)
install(EXPORT VKBOTTargets
    NAMESPACE VKBOT::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VKBOT
)

# ---------- примеры ----------
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
