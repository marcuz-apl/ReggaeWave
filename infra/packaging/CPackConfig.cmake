# ReggaeWave CPack Release Configuration

set(CPACK_PACKAGE_NAME "ReggaeWave")
set(CPACK_PACKAGE_VENDOR "ReggaeWave")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert any music genre into authentic Reggae")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ReggaeWave")

if(APPLE)
    # macOS DMG & App Bundle
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "ReggaeWave Installer")
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/infra/packaging/assets/reggaewave.icns")
    set(CPACK_BUNDLE_NAME "ReggaeWave")
    set(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/infra/packaging/macOS/Info.plist")
elseif(WIN32)
    # Windows 11 NSIS / MSI Installer
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "ReggaeWave Desktop")
    set(CPACK_NSIS_PACKAGE_NAME "ReggaeWave")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
else()
    # Linux Debian / TGZ / AppImage
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ReggaeWave Team <team@reggaewave.internal>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2, libx11-6, libfreetype6")
endif()

include(CPack)
