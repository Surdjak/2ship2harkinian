add_compile_definitions(CONTROLLERBUTTONS_T=uint32_t)

# Give the standalone OoT build a configuration-specific, predictable output path.
function(set_oot_zapd_output)
    set_property(TARGET ZAPD PROPERTY SOURCES "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../CMake/OotZapdMain.cpp")
    foreach(config Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${config}" upper_config)
        set_target_properties(ZAPD PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${upper_config} "${CMAKE_BINARY_DIR}/bin/${config}"
            RUNTIME_OUTPUT_NAME_${upper_config} "zapd-oot")
    endforeach()
endfunction()
cmake_language(DEFER CALL set_oot_zapd_output)
