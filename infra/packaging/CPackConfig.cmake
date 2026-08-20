# ReggaeWave CPack Release Configuration

set(CPACK_PACKAGE_NAME "ReggaeWave")
set(CPACK_PACKAGE_VENDOR "ReggaeWave")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert any music genre into authentic Reggae")
set(CPACK_PACKAGE_VERSION_MAJOR ${REGGAEWAVE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${REGGAEWAVE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${REGGAEWAVE_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${REGGAEWAVE_PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ReggaeWave")

if(APPLE)
    # macOS DMG & App Bundle
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "ReggaeWave Installer")
    set(CPACK_BUNDLE_NAME "ReggaeWave")
elseif(WIN32)
    # Windows 11 NSIS / MSI Installer
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "ReggaeWave Desktop")
    set(CPACK_NSIS_PACKAGE_NAME "ReggaeWave")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
else()
    # Linux Debian (.deb) and RedHat/RPM (.rpm)
    set(CPACK_GENERATOR "DEB;RPM")
    
    # Debian Configuration
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ReggaeWave Team <team@reggaewave.internal>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2t64 | libasound2, libx11-6, libfreetype6, libgtk-3-0t64 | libgtk-3-0")
    
    # RPM Configuration
    set(CPACK_RPM_PACKAGE_SUMMARY "Convert any music genre into authentic Reggae")
    set(CPACK_RPM_PACKAGE_LICENSE "Proprietary")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
    set(CPACK_RPM_PACKAGE_AUTOREQPROV OFF)
endif()

include(CPack)

