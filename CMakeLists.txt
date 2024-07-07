cmake_minimum_required(VERSION 2.8.12)
project(RunnerSymbols)

set(CMAKE_PROJECT_VERSION  "1.1.0")
set(CMAKE_PROJECT_AUTHOR "Dominik P. Schreiber")
set(CMAKE_PROJECT_HOMEPAGE_URL "github.com/domschrei/krunner-symbols")

set(KF6_MIN_VERSION 6.0.0)
find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH}
)

find_package (Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets Core Quick Core5Compat)
find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS I18n Service Runner TextWidgets)
find_package (PlasmaQuick  ${KF6_MIN_VERSION} REQUIRED )

set(CPACK_PACKAGE_NAME "krunner-symbols")
set(CPACK_PACKAGE_VENDOR "dominikschreiber.de")
set(CPACK_PACKAGE_CONTACT "dev@dominikschreiber.de")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Plugin for Plasma 6 krunner to retrieve arbitrary symbols or macros")
set(CPACK_VERBATIM_VARIABLES true)

if (CPACK_DEBIAN)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CMAKE_PROJECT_AUTHOR}")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.deb")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libkf5runner5") # TODO: needs to be updated once Debian packages Plasma 6?
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst;${CMAKE_CURRENT_SOURCE_DIR}/postrm")
    unset(CPACK_DEBIAN CACHE)
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(krunner_symbols_SRCS symbols.cpp)
add_library(krunner_symbols MODULE ${krunner_symbols_SRCS})
target_link_libraries(krunner_symbols KF6::Runner Qt6::Widgets KF6::I18n KF6::Service Plasma::Plasma Qt6::Core5Compat)

install(TARGETS krunner_symbols DESTINATION ${LOCATION_PLUGIN}/kf6/krunner)
install(FILES krunner-symbolsrc DESTINATION ${LOCATION_CONFIG})
install(DIRECTORY krunner-symbols-unicode DESTINATION ${LOCATION_CONFIG})
