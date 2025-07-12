# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

# Target: static library
qt_add_qml_module(windowsapphelper_static
    STATIC
    URI org.filotimoproject.windowsapphelper
    VERSION 1.0
    QML_FILES
        contents/ui/Main.qml
)

target_sources(windowsapphelper_static PUBLIC
    WindowsAppHelper.cpp
)

target_link_libraries(windowsapphelper_static PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Widgets
    KF6::I18n
    KF6::CoreAddons
    KF6::KIOCore
    KF6::KIOWidgets
)
target_include_directories(windowsapphelper_static PUBLIC ${CMAKE_BINARY_DIR})

# Target: main executable
add_executable(windowsapphelper main.cpp)
target_link_libraries(windowsapphelper PUBLIC windowsapphelper_static windowsapphelper_staticplugin)
install(FILES app_db.json DESTINATION ${KDE_INSTALL_DATADIR}/windowsapphelper)
install(TARGETS windowsapphelper ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
