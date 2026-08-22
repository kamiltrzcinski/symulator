# vcpkg's dynamically-linked Qt6 relies on qt.conf / QT_PLUGIN_PATH for plugin
# discovery (platforms, imageformats, ...) — this is separate from, and not
# covered by, ordinary shared-library linking (RPATH on Linux/macOS,
# VCPKG_APPLOCAL_DEPS on Windows). Nothing in the vcpkg qtbase port installs a
# qt.conf for consumer executables (only for Qt's own dev tools), so every Qt
# GUI/test binary this project builds needs one of the two helpers below.

# For ctest-driven binaries: set QT_PLUGIN_PATH (and QT_QPA_PLATFORM=offscreen,
# since none of our test binaries need a real display) via test properties.
# Call after add_test().
function(symulator_set_qt_test_environment target)
    set_tests_properties(${target} PROPERTIES
        ENVIRONMENT_MODIFICATION
            "QT_QPA_PLATFORM=set:offscreen;QT_PLUGIN_PATH=set:${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS};PATH=path_list_prepend:${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}"
    )
endfunction()

# For interactively-run GUI binaries (client, editor, dev tools): write a
# qt.conf next to the built executable with an absolute Prefix, so a developer
# can run it directly from the build tree without exporting QT_PLUGIN_PATH by
# hand. Safe to hardcode an absolute, machine-specific path here — the file
# only ever lives in the gitignored build directory, regenerated per machine
# at configure time. Call after the target is created.
#
# Every QT6_INSTALL_* key is written explicitly, not just Prefix: a qt.conf
# that only sets Prefix makes Qt fall back to its own compiled-in *relative*
# default for any key it doesn't find (e.g. "plugins"), not to vcpkg's actual
# nested layout ("Qt6/plugins") — silently breaking plugin discovery instead
# of fixing it. Mirrors vcpkg's own tools/Qt6/bin/qt.conf, which sets the same
# keys for Qt's dev tools (moc/uic/rcc).
function(symulator_deploy_qt_conf target)
    file(GENERATE
        OUTPUT "$<TARGET_FILE_DIR:${target}>/qt.conf"
        CONTENT
"[Paths]
Prefix=${QT6_INSTALL_PREFIX}
Binaries=${QT6_INSTALL_BINS}
Libraries=${QT6_INSTALL_LIBS}
Plugins=${QT6_INSTALL_PLUGINS}
Qml2Imports=${QT6_INSTALL_QML}
Translations=${QT6_INSTALL_TRANSLATIONS}
Data=${QT6_INSTALL_DATA}
ArchData=${QT6_INSTALL_ARCHDATA}
Headers=${QT6_INSTALL_HEADERS}
"
    )
endfunction()
