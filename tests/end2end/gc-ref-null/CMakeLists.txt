cmake_minimum_required(VERSION 3.14)

set(TEST_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/run-test.sh")

# Make script executable
file(CHMOD "${TEST_SCRIPT}" PERMISSIONS
     OWNER_READ OWNER_WRITE OWNER_EXECUTE
     GROUP_READ GROUP_EXECUTE
     WORLD_READ WORLD_EXECUTE)

# Non-GC mode tests (only when WASM_ENABLE_GC=0)
# These tests use non-GC ref.null syntax and require non-GC iwasm
if(NOT WASM_ENABLE_GC)
    add_test(
        NAME e2e_gc_ref_null_non_gc_funcref
        COMMAND ${TEST_SCRIPT} non-gc-funcref.wat false success success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_gc_ref_null_non_gc_funcref PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;gc;ref-null"
    )

    add_test(
        NAME e2e_gc_ref_null_non_gc_externref
        COMMAND ${TEST_SCRIPT} non-gc-externref.wat false success success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_gc_ref_null_non_gc_externref PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;gc;ref-null"
    )

    add_test(
        NAME e2e_gc_ref_null_non_gc_invalid
        COMMAND ${TEST_SCRIPT} non-gc-invalid.wat false fail skip
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_gc_ref_null_non_gc_invalid PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;gc;ref-null;negative"
    )

    message(STATUS "  ref.null tests: Non-GC mode tests enabled")
endif()

# GC mode tests (only when WASM_ENABLE_GC=1)
if(WASM_ENABLE_GC)
    add_test(
        NAME e2e_gc_ref_null_gc_type_index
        COMMAND ${TEST_SCRIPT} gc-type-index.wat true success success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_gc_ref_null_gc_type_index PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;gc;ref-null"
    )

    add_test(
        NAME e2e_gc_ref_null_gc_abstract_type
        COMMAND ${TEST_SCRIPT} gc-abstract-type.wat true success success
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_gc_ref_null_gc_abstract_type PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;gc;ref-null"
    )

    add_test(
        NAME e2e_gc_ref_null_gc_out_of_range
        COMMAND ${TEST_SCRIPT} gc-out-of-range.wat true fail skip
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(e2e_gc_ref_null_gc_out_of_range PROPERTIES
        ENVIRONMENT "${E2E_TEST_ENV}"
        LABELS "e2e;gc;ref-null;negative"
    )

    message(STATUS "  ref.null tests: GC mode tests enabled")
    message(STATUS "    Note: GC tests require WABT with GC support to compile WAT files")
endif()
