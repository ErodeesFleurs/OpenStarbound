# Based on vcpkg d015e31e90838a4c9dfa3eed45979bc70d9357fc ports/gperf.
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_BUILD_TYPE release) # tool only

vcpkg_download_distfile(ARCHIVE
    URLS
        "https://ftpmirror.gnu.org/gnu/gperf/gperf-${VERSION}.tar.gz"
        "https://ftp.gnu.org/pub/gnu/gperf/gperf-${VERSION}.tar.gz"
    FILENAME gperf-${VERSION}.tar.gz
    SHA512 246b75b8ce7d77d6a8725cd15f1cf2e68da404812573af1d5bf32dbe6ad4228f48757baefc77bcb1f5597c2397043c04d31d8a04ab507bfa7a80f85e1ab6045f
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_make_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    AUTORECONF
    OPTIONS_RELEASE
        "--bindir=\\\${prefix}/tools/${PORT}"  # legacy from vendored CMake build
)

vcpkg_make_install()

# vcpkg-make can stage ARM64 gperf under DESTDIR/usr/local.
if(VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
    include("${CMAKE_CURRENT_LIST_DIR}/../normalize-arm64-vcpkg-make-install.cmake")
endif()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
