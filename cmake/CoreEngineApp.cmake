function(coreengine_configure_app target)
  cmake_parse_arguments(APP "" "NAME;IDENTIFIER;OUTPUT_NAME" "" ${ARGN})

  set(icon_dir "${CMAKE_SOURCE_DIR}/assets/icon")
  set(platform_dir "${CMAKE_SOURCE_DIR}/platform")

  if(NOT ANDROID)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME "${APP_OUTPUT_NAME}")
  endif()

  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra)
  endif()

  if(WIN32)
    set(APP_ICON_FILE "${icon_dir}/icon.ico")
    configure_file("${platform_dir}/windows/app.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/${target}.rc" @ONLY)
    target_sources(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${target}.rc")
    set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE ON)
  elseif(IOS)
    set(assets "${platform_dir}/ios/Assets.xcassets")
    target_sources(${target} PRIVATE "${assets}")
    set_source_files_properties("${assets}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST "${platform_dir}/ios/Info.plist.in"
      MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
      MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_IDENTIFIER}"
      MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
      XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${APP_IDENTIFIER}"
      XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
      XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    )
  elseif(APPLE)
    set(icns "${icon_dir}/icon.icns")
    target_sources(${target} PRIVATE "${icns}")
    set_source_files_properties("${icns}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
      MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_IDENTIFIER}"
      MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
      MACOSX_BUNDLE_ICON_FILE "icon.icns"
      XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${APP_IDENTIFIER}"
    )
  elseif(UNIX AND NOT ANDROID)
    target_link_options(${target} PRIVATE -static-libstdc++ -static-libgcc)
  endif()
endfunction()
