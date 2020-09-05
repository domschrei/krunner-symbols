cmake_minimum_required(VERSION 2.8.12)
project(RunnerSymbols)

set(CMAKE_PROJECT_VERSION  "1.1.0")
set(CMAKE_PROJECT_AUTHOR "Dominik P. Schreiber")
set(CMAKE_PROJECT_HOMEPAGE_URL "github.com/domschrei/krunner-symbols")

find_package(ECM 5.12.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH}
)

find_package (Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets Core Quick)
find_package (KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Service Runner TextWidgets PlasmaQuick)

set(CPACK_PACKAGE_NAME "krunner-symbols")
set(CPACK_PACKAGE_VENDOR "dominikschreiber.de")
set(CPACK_PACKAGE_CONTACT "dev@dominikschreiber.de")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Plugin for Plasma 5 krunner to retrieve arbitrary symbols or macros")
set(CPACK_VERBATIM_VARIABLES true)

if (CPACK_DEBIAN)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CMAKE_PROJECT_AUTHOR}")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.deb")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libkf5runner5")
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
target_link_libraries(krunner_symbols KF5::Runner Qt5::Widgets KF5::I18n KF5::Service KF5::Plasma)

install(TARGETS krunner_symbols DESTINATION ${LOCATION_PLUGIN})
install(FILES plasma-runner-symbols.desktop DESTINATION ${LOCATION_DESKTOP})
install(FILES krunner-symbolsrc DESTINATION ${LOCATION_CONFIG})
install(FILES krunner-symbols-full-unicode-index DESTINATION ${LOCATION_CONFIG})
