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
