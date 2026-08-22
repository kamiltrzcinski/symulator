# Same as vcpkg's stock x64-windows triplet (dynamic), except libpqxx is
# forced static. libpqxx's `zview` class publicly inherits `std::string_view`
# and is annotated PQXX_LIBEXPORT, which becomes `__declspec(dllimport)` once
# a dynamic libpqxx propagates PQXX_SHARED to consumers. MSVC then re-emits
# std::string_view's inherited members as imported symbols, colliding with
# the normal weak template instantiations already present in any translation
# unit that includes <string_view> — LNK2005 "already defined" at link time.
# GCC/Clang don't have this failure mode (visibility attributes, not
# dllimport/dllexport, so Linux/macOS -dynamic triplets are unaffected and
# intentionally not overridden here). Static libpqxx sidesteps PQXX_SHARED
# entirely.
include("${CMAKE_CURRENT_LIST_DIR}/../3rdParty/vcpkg/triplets/x64-windows.cmake")

if(PORT MATCHES "^libpqxx$")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()
