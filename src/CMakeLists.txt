# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

# Target: static library
qt_add_qml_module(appcompatibilityhelper_static
    STATIC
    URI org.filotimoproject.appcompatibilityhelper
    VERSION 1.0
    QML_FILES
        contents/ui/Main.qml
)

target_sources(appcompatibilityhelper_static PUBLIC
    ICompatibilityHelper.cpp
    CompatibilityHelperFactory.cpp
    WindowsCompatibilityHelper.cpp
    RpmCompatibilityHelper.cpp
    DebCompatibilityHelper.cpp
)

target_link_libraries(appcompatibilityhelper_static PUBLIC
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
target_include_directories(appcompatibilityhelper_static PUBLIC ${CMAKE_BINARY_DIR})

# Target: main executable
add_executable(appcompatibilityhelper main.cpp)
target_link_libraries(appcompatibilityhelper PUBLIC appcompatibilityhelper_static appcompatibilityhelper_staticplugin)
install(FILES app_db.json DESTINATION ${KDE_INSTALL_DATADIR}/appcompatibilityhelper)
install(TARGETS appcompatibilityhelper ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
