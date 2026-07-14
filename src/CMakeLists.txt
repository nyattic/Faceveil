set(REDACTLY_APP_OPTIONS)
set(REDACTLY_APP_RESOURCES)
if(APPLE)
    list(APPEND REDACTLY_APP_OPTIONS MACOSX_BUNDLE)
    set(APP_ICON_MACOS "${PROJECT_SOURCE_DIR}/assets/Redactly.icns")
    if(EXISTS "${APP_ICON_MACOS}")
        set_source_files_properties("${APP_ICON_MACOS}" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
        list(APPEND REDACTLY_APP_RESOURCES "${APP_ICON_MACOS}")
    endif()
elseif(WIN32)
    list(APPEND REDACTLY_APP_OPTIONS WIN32)
endif()

add_library(redactly_core STATIC
    ImageIo.cpp
    ImageScanner.cpp
    Mosaic.cpp
    OnnxGraphPatch.cpp
    OrtAcceleration.cpp
    OutputPlan.cpp
    PlateDetector.cpp
    ProcessorWorker.cpp
    SceneCut.cpp
    ScrfdFaceDetector.cpp
    Tracking.cpp
    VideoIo.cpp
    VideoProcessor.cpp
    ${PROJECT_SOURCE_DIR}/include/redactly/DetectionGeometry.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/Detector.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/FaceDetection.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ImageIo.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ImageScanner.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/Mosaic.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/OnnxGraphPatch.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/OrderedParallel.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/OrtAcceleration.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/OutputPlan.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/PathSafety.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/PathUtil.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/PlateDetector.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ProcessorWorker.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ReviewTypes.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/SceneCut.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ScrfdFaceDetector.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/Tracking.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/VideoIo.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/VideoProcessor.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/VideoReviewTypes.hpp
)

target_include_directories(redactly_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(redactly_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    ${OpenCV_LIBS}
    onnxruntime::onnxruntime
    spdlog::spdlog
)

if(REDACTLY_EXIV2_TARGET)
    target_link_libraries(redactly_core PUBLIC ${REDACTLY_EXIV2_TARGET})
    target_compile_definitions(redactly_core PUBLIC REDACTLY_HAVE_EXIV2)
    message(STATUS "Redactly: metadata preservation enabled (${REDACTLY_EXIV2_TARGET})")
else()
    message(WARNING "exiv2 not found; building without metadata preservation.")
endif()

qt_add_executable(Redactly ${REDACTLY_APP_OPTIONS}
    main.cpp
    MainWindow.cpp
    ModelCatalog.cpp
    ModelDownloader.cpp
    ReviewDialog.cpp
    SettingsDialog.cpp
    Theme.cpp
    UpdateChecker.cpp
    VideoReviewDialog.cpp
    ${PROJECT_SOURCE_DIR}/include/redactly/MainWindow.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ModelCatalog.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ModelDownloader.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/ReviewDialog.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/SettingsDialog.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/Theme.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/UpdateChecker.hpp
    ${PROJECT_SOURCE_DIR}/include/redactly/VideoReviewDialog.hpp
    ${REDACTLY_APP_RESOURCES}
)

if(WIN32)
    target_sources(Redactly PRIVATE ${PROJECT_SOURCE_DIR}/assets/Redactly.rc)
endif()

set_target_properties(Redactly PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_compile_definitions(Redactly PRIVATE REDACTLY_VERSION="${PROJECT_VERSION}")
target_link_libraries(Redactly PRIVATE
    redactly_core
    Qt6::Widgets
    Qt6::Network
)

if(TARGET Qt6::Svg)
    target_link_libraries(Redactly PRIVATE Qt6::Svg)
    target_compile_definitions(Redactly PRIVATE REDACTLY_HAVE_SVG)
    message(STATUS "Redactly: SVG settings icon enabled")
else()
    message(STATUS "Redactly: Qt Svg not found; using a text glyph for the settings icon")
endif()

if(Qt6LinguistTools_FOUND)
    qt_add_translations(Redactly
        TS_FILES ${PROJECT_SOURCE_DIR}/translations/redactly_ko.ts
                 ${PROJECT_SOURCE_DIR}/translations/redactly_ja.ts
                 ${PROJECT_SOURCE_DIR}/translations/redactly_zh_CN.ts
    )
else()
    message(WARNING "Qt LinguistTools not found; building without embedded translations (English only).")
endif()

redactly_harden(redactly_core)
redactly_harden(Redactly)

if(APPLE)
    set_target_properties(Redactly PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.redactly.app"
        MACOSX_BUNDLE_BUNDLE_NAME "Redactly"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "Redactly.icns"
    )
endif()

install(TARGETS Redactly
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(UNIX AND NOT APPLE)
    install(FILES ${PROJECT_SOURCE_DIR}/assets/redactly.desktop DESTINATION share/applications)
    install(FILES ${PROJECT_SOURCE_DIR}/assets/redactly-512.png
        DESTINATION share/icons/hicolor/512x512/apps RENAME redactly.png)
endif()
