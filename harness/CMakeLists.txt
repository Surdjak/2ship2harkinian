# Differential testing harness, compiled into the 2ship target. SoH builds the same core with its own
# adapter from harness/soh. See harness/README.md.
target_sources(2ship PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Harness.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/2ship/Harness2Ship.cpp
)
target_include_directories(2ship PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core)
