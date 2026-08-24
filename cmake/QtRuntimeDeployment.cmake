# vcpkg's dynamically-linked Qt6 relies on qt.conf / QT_PLUGIN_PATH for plugin
# discovery (platforms, imageformats, ...) — this is separate from, and not
# covered by, ordinary shared-library linking (RPATH on Linux/macOS,
# VCPKG_APPLOCAL_DEPS on Windows). Nothing in the vcpkg qtbase port installs a
# qt.conf for consumer executables (only for Qt's own dev tools), so every Qt
# GUI/test binary this project builds needs one of the two helpers below.
#
# QT6_INSTALL_PREFIX/QT6_INSTALL_PLUGINS/QT6_INSTALL_BINS (from Qt's own
# QtInstallPaths.cmake) only describe the Release-config layout. vcpkg's
# Windows triplets additionally install a full Debug-config copy side by
# side, under "<prefix>/debug/..." with a "d" suffix on debug binaries (e.g.
# "Qt6/plugins/platforms/qoffscreend.dll" vs. the Release "qoffscreen.dll")
# — vcpkg has no way to express "Debug plugins live here" in Qt's own
# install-path variables, since that split is a vcpkg packaging convention,
# not something Qt's build system models. On Windows/MSVC this isn't
# cosmetic: loading a Release plugin into a Debug Qt/CRT process is an ABI
# mismatch that QPluginLoader can't paper over, and confirmed via CI log
# inspection to be exactly why every test constructing QApplication hung
# until timeout (QT_PLUGIN_PATH pointed at the Release qoffscreen.dll while
# the test binary itself is a Debug build). Linux/macOS have the same
# debug/ split on disk but don't need this correction — no separate debug
# CRT/ABI, so the Release .so loads into a Debug process without issue
# (verified: local Linux Debug builds already worked before this fix).
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_symulator_qt_runtime_prefix "${QT6_INSTALL_PREFIX}/debug")
else()
    set(_symulator_qt_runtime_prefix "${QT6_INSTALL_PREFIX}")
endif()

# Printed once at configure time (not per-target) so any CI "Configure" log
# always shows the resolved values without needing a separate run to fail
# first — useful when diagnosing "platform plugin not found"-class issues,
# since QT6_INSTALL_PLUGINS's actual on-disk layout is asserted here, not
# just assumed.
message(STATUS "Qt6 runtime paths: prefix=${_symulator_qt_runtime_prefix} "
    "plugins=${_symulator_qt_runtime_prefix}/${QT6_INSTALL_PLUGINS} "
    "bins=${_symulator_qt_runtime_prefix}/${QT6_INSTALL_BINS}")
if(EXISTS "${_symulator_qt_runtime_prefix}/${QT6_INSTALL_PLUGINS}/platforms")
    file(GLOB _symulator_qt_platform_plugins
        "${_symulator_qt_runtime_prefix}/${QT6_INSTALL_PLUGINS}/platforms/*")
    message(STATUS "Qt6 platform plugins found: ${_symulator_qt_platform_plugins}")
else()
    message(WARNING "Qt6 platforms plugin directory does not exist: "
        "${_symulator_qt_runtime_prefix}/${QT6_INSTALL_PLUGINS}/platforms")
endif()

# For ctest-driven binaries: set QT_PLUGIN_PATH (and QT_QPA_PLATFORM=offscreen,
# since none of our test binaries need a real display) via test properties.
# Call after add_test().
#
# TIMEOUT guards against silent multi-hour hangs instead of a fast, readable
# failure — e.g. a platform plugin that fails to load without an explicit
# error, or (Windows Debug CRT specifically) an assert()/uncaught exception
# popping a modal "Abort/Retry/Ignore" dialog that nobody on a headless CI
# runner can click. All tests using this helper are plain unit tests (no
# sleeps/network/IO) that finish in well under a second locally, so 120s
# leaves generous headroom for a slow/cold CI runner while still failing fast.
#
# QT_DEBUG_PLUGINS=1 is a temporary diagnostic: on Windows CI, every test that
# constructs a QApplication (needs the "offscreen" platform plugin) hangs
# until TIMEOUT, while QCoreApplication-only tests pass — this makes Qt log
# every plugin candidate it inspects and why it accepts/rejects it, so the
# next hang is actually explainable instead of a silent timeout.
function(symulator_set_qt_test_environment target)
    set_tests_properties(${target} PROPERTIES
        ENVIRONMENT_MODIFICATION
            "QT_QPA_PLATFORM=set:offscreen;QT_PLUGIN_PATH=set:${_symulator_qt_runtime_prefix}/${QT6_INSTALL_PLUGINS};PATH=path_list_prepend:${_symulator_qt_runtime_prefix}/${QT6_INSTALL_BINS};QT_DEBUG_PLUGINS=set:1"
        TIMEOUT 120
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
#
# On Windows, qt_add_executable() funnels every GUI target's output into one
# shared CMAKE_RUNTIME_OUTPUT_DIRECTORY (Qt needs same-directory DLL/plugin
# placement there; Linux/macOS don't, since RPATH covers it, so each target
# keeps its own output directory). Writing qt.conf per target would then mean
# multiple file(GENERATE) calls targeting the exact same path, which CMake's
# generate step rejects even when the content is identical. Detect the shared
# case via CMAKE_RUNTIME_OUTPUT_DIRECTORY and write once for the whole build.
function(symulator_deploy_qt_conf target)
    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        get_property(_symulator_qt_conf_written GLOBAL PROPERTY _symulator_qt_conf_written)
        if(_symulator_qt_conf_written)
            return()
        endif()
        set_property(GLOBAL PROPERTY _symulator_qt_conf_written TRUE)
        set(_qt_conf_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
        set(_qt_conf_dir "$<TARGET_FILE_DIR:${target}>")
    endif()

    file(GENERATE
        OUTPUT "${_qt_conf_dir}/qt.conf"
        CONTENT
"[Paths]
Prefix=${_symulator_qt_runtime_prefix}
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
