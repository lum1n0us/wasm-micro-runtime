cmake_minimum_required(VERSION 3.14)

set(TEST_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/run-test.sh")

# Make script executable
file(CHMOD "${TEST_SCRIPT}" PERMISSIONS
     OWNER_READ OWNER_WRITE OWNER_EXECUTE
     GROUP_READ GROUP_EXECUTE
     WORLD_READ WORLD_EXECUTE)

# Test matrix for AOT GC compatibility
# Goal: GC iwasm should load both GC and non-GC AOT (backward compatibility)
#       Non-GC iwasm should only load non-GC AOT

if(WASM_ENABLE_GC)
    # === GC Build Tests ===

    # Test 1: GC iwasm + GC AOT (full GC support)
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

    # Test 2: GC iwasm + non-GC AOT (backward compatibility)
    # Currently BLOCKED by AOT loader bug - see README.md
    add_test(
        NAME e2e_aot_gc_compat_gc_iwasm_nogc_aot_simple
        COMMAND ${TEST_SCRIPT} simple-no-table.wat false success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_aot_gc_compat_gc_iwasm_nogc_aot_simple PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;aot;gc-compat;backward-compat"
        DISABLED TRUE  # Blocked by AOT loader bug
    )

    add_test(
        NAME e2e_aot_gc_compat_gc_iwasm_nogc_aot_with_table
        COMMAND ${TEST_SCRIPT} simple-with-table.wat false success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_aot_gc_compat_gc_iwasm_nogc_aot_with_table PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;aot;gc-compat;backward-compat;regression"
        DISABLED TRUE  # Blocked by AOT loader bug - table init_expr issue
    )

    message(STATUS "  AOT GC compatibility tests:")
    message(STATUS "    ✅ GC iwasm + GC AOT")
    message(STATUS "    🔴 GC iwasm + non-GC AOT (backward compat - DISABLED, blocked by bug)")
else()
    # === Non-GC Build Tests ===

    # Test: Non-GC iwasm + non-GC AOT (baseline)
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

    message(STATUS "  AOT GC compatibility tests:")
    message(STATUS "    ✅ Non-GC iwasm + non-GC AOT")
endif()
