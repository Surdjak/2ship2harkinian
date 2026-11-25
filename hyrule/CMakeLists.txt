# Hyrule (Ocarina of Time) sources. They are compiled into the 2ship target rather than a separate
# library so they share its include paths, compile definitions and flags with mm/.
file(GLOB_RECURSE hyrule__ RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c" "*.cpp" "*.h" "*.hpp")
# Create source groups that match the real file directory paths
foreach(source IN LISTS hyrule__)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_adjusted "hyrule/${source_path}")
    source_group("${source_path_adjusted}" FILES "${source}")
endforeach()

if (hyrule__)
    target_sources(2ship PRIVATE ${hyrule__})
endif()
target_include_directories(2ship PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

################################################################################
# OoT asset extraction
################################################################################
# ZAPD and OTRExporter choose their game at compile time, and the main tree builds them for MM.
# Build a second, standalone copy of OTRExporter's project, which defaults to OoT.
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows|Linux|Darwin")
    return()
endif()

include(ExternalProject)
set(OOT_ZAPD "${CMAKE_BINARY_DIR}/ZAPD_Oot/bin/$<CONFIG>/zapd-oot${CMAKE_EXECUTABLE_SUFFIX}")
set(OOT_ZAPD_ARGS
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_PROJECT_OTRExporter_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/OotZapdOutput.cmake
    -DINCLUDE_MPQ_SUPPORT=ON
    -DGAME_STR=OOT)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND OOT_ZAPD_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()
if(VCPKG_TARGET_TRIPLET)
    list(APPEND OOT_ZAPD_ARGS -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET})
endif()
if(APPLE)
    string(REPLACE ";" "|" OOT_ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}")
    list(APPEND OOT_ZAPD_ARGS "-DCMAKE_OSX_ARCHITECTURES=${OOT_ARCHITECTURES}"
        "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

ExternalProject_Add(ZAPD_Oot
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/OTRExporter
    BINARY_DIR ${CMAKE_BINARY_DIR}/ZAPD_Oot
    LIST_SEPARATOR |
    CMAKE_ARGS ${OOT_ZAPD_ARGS}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG> --target ZAPD
    BUILD_BYPRODUCTS "${OOT_ZAPD}"
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
    EXCLUDE_FROM_ALL TRUE
)

# Stage both the extractor and its inputs in the build tree and the distributable.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/stage-oot-$<CONFIG>.cmake" CONTENT "
    file(MAKE_DIRECTORY \"$<TARGET_FILE_DIR:2ship>\")
    file(COPY \"${OOT_ZAPD}\" DESTINATION \"$<TARGET_FILE_DIR:2ship>\")
    file(COPY \"${CMAKE_CURRENT_SOURCE_DIR}/assets/extractor/\" \"${CMAKE_CURRENT_SOURCE_DIR}/assets/xml\"
        DESTINATION \"$<TARGET_FILE_DIR:2ship>/oot-assets\")
")
add_custom_target(StageOotExtractor
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/stage-oot-$<CONFIG>.cmake"
    DEPENDS ZAPD_Oot
    VERBATIM)
add_dependencies(2ship StageOotExtractor)
target_compile_definitions(2ship PRIVATE ENABLE_OOT_EXTRACTOR)
get_target_property(STORM_INCLUDES storm INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(2ship PRIVATE "${STORM_INCLUDES}")
# libultraship links storm itself when it has MPQ support (whole-archive outside Windows and iOS, which a second plain
# link would conflict with).
if(NOT INCLUDE_MPQ_SUPPORT)
    target_link_libraries(2ship PRIVATE storm)
endif()

set(OOT_INSTALL_DIR .)
if(APPLE)
    set(OOT_INSTALL_DIR ../Resources)
endif()
install(PROGRAMS "${OOT_ZAPD}" DESTINATION "${OOT_INSTALL_DIR}" COMPONENT 2s2h)
install(DIRECTORY assets/extractor/ DESTINATION "${OOT_INSTALL_DIR}/oot-assets" COMPONENT 2s2h)
install(DIRECTORY assets/xml DESTINATION "${OOT_INSTALL_DIR}/oot-assets" COMPONENT 2s2h)

# Only N64 NTSC 1.1 inputs are imported so far.
set(OOT_ROM_PATH "" CACHE FILEPATH "OoT N64 NTSC 1.1 ROM used by ExtractOotAssets")
set(OOT_ROM_VERSION N64_NTSC_11)
set(OOT_ROM_SHA1 d3ecb253776cd847a5aa63d859d8c89a2f37b364)
set(OOT_EXTRACT_DIR ${CMAKE_BINARY_DIR}/oot)

if (NOT OOT_ROM_PATH)
    add_custom_target(ExtractOotAssets
        COMMAND ${CMAKE_COMMAND} -E echo "Configure with -DOOT_ROM_PATH=<OoT N64 NTSC 1.1 ROM> first."
        COMMAND ${CMAKE_COMMAND} -E false
    )
else()
    file(SHA1 ${OOT_ROM_PATH} OOT_ROM_ACTUAL_SHA1)
    if (NOT OOT_ROM_ACTUAL_SHA1 STREQUAL OOT_ROM_SHA1)
        message(FATAL_ERROR "OOT_ROM_PATH is not an OoT N64 NTSC 1.1 ROM (SHA-1 ${OOT_ROM_ACTUAL_SHA1})")
    endif()
    file(MAKE_DIRECTORY ${OOT_EXTRACT_DIR})

    # Writes ${OOT_EXTRACT_DIR}/oot.o2r with every path under oot/ so it can sit beside mm.o2r. The
    # exporter adds to an existing archive, so the old one is removed first.
    add_custom_target(ExtractOotAssets
        COMMAND ${CMAKE_COMMAND} -E rm -rf oot.o2r assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/extractor assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/xml assets/xml
        COMMAND "${OOT_ZAPD}" ed
            -i assets/xml/${OOT_ROM_VERSION}
            -b ${OOT_ROM_PATH}
            -fl assets/filelists
            -gsf 0
            -rconf assets/Config_${OOT_ROM_VERSION}.xml
            -se OTR
            --otrfile oot.o2r
            --pathRoot oot
            --convertToMm
            --portVer ${CMAKE_PROJECT_VERSION}
            -o placeholder
            -osf placeholder
        # The game loads oot.o2r from its own folder (hyrule/HyruleScenes.cpp).
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:2ship>"
        COMMAND ${CMAKE_COMMAND} -E copy oot.o2r $<TARGET_FILE_DIR:2ship>/oot.o2r
        WORKING_DIRECTORY ${OOT_EXTRACT_DIR}
        COMMENT "Extracting OoT assets into ${OOT_EXTRACT_DIR}/oot.o2r"
        VERBATIM
    )
    add_dependencies(ExtractOotAssets ZAPD_Oot)
endif()
