cmake_minimum_required(VERSION 3.14)

set(TEST_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/run-test.sh")

# Make script executable
file(CHMOD "${TEST_SCRIPT}" PERMISSIONS
     OWNER_READ OWNER_WRITE OWNER_EXECUTE
     GROUP_READ GROUP_EXECUTE
     WORLD_READ WORLD_EXECUTE)

# Test matching configurations only:
# - GC iwasm + GC AOT (when WASM_ENABLE_GC=1)
# - Non-GC iwasm + non-GC AOT (when WASM_ENABLE_GC=0)

if(WASM_ENABLE_GC)
    # GC build: Test GC iwasm with GC AOT
    add_test(
        NAME e2e_aot_gc_compat_gc_aot_simple
        COMMAND ${TEST_SCRIPT} simple-no-table.wat true success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_aot_gc_compat_gc_aot_simple PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;aot;gc-compat"
    )

    add_test(
        NAME e2e_aot_gc_compat_gc_aot_with_table
        COMMAND ${TEST_SCRIPT} simple-with-table.wat true success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_aot_gc_compat_gc_aot_with_table PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;aot;gc-compat"
    )

    message(STATUS "  AOT GC compatibility tests: GC iwasm + GC AOT")
else()
    # Non-GC build: Test non-GC iwasm with non-GC AOT
    add_test(
        NAME e2e_aot_gc_compat_nogc_aot_simple
        COMMAND ${TEST_SCRIPT} simple-no-table.wat false success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_aot_gc_compat_nogc_aot_simple PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;aot;gc-compat"
    )

    add_test(
        NAME e2e_aot_gc_compat_nogc_aot_with_table
        COMMAND ${TEST_SCRIPT} simple-with-table.wat false success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_aot_gc_compat_nogc_aot_with_table PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;aot;gc-compat"
    )

    message(STATUS "  AOT GC compatibility tests: Non-GC iwasm + non-GC AOT")
endif()
