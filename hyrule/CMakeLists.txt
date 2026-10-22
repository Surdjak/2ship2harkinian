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
include(ExternalProject)
ExternalProject_Add(ZAPD_Oot
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/OTRExporter
    BINARY_DIR ${CMAKE_BINARY_DIR}/ZAPD_Oot
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DGAME_STR=OOT
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target ZAPD
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
    EXCLUDE_FROM_ALL TRUE
)

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
        COMMAND ${CMAKE_BINARY_DIR}/ZAPD_Oot/ZAPD/ZAPD.out ed
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
        COMMAND ${CMAKE_COMMAND} -E copy oot.o2r $<TARGET_FILE_DIR:2ship>/oot.o2r
        WORKING_DIRECTORY ${OOT_EXTRACT_DIR}
        COMMENT "Extracting OoT assets into ${OOT_EXTRACT_DIR}/oot.o2r"
        VERBATIM
    )
    add_dependencies(ExtractOotAssets ZAPD_Oot)
endif()
