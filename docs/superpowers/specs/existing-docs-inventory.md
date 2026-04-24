# Existing WAMR Documentation Inventory

## Core Guides
- README.md - Project overview, key features, getting started
- doc/build_wamr.md - Build system configuration, WAMR_BUILD_* flags
- doc/embed_wamr.md - Embedding API guide
- doc/export_native_api.md - Native function registration
- doc/build_wasm_app.md - Wasm application building

## Specialized Topics
- doc/source_debugging.md - Debugging workflows
- doc/memory_tune.md - Memory usage tuning
- doc/perf_tune.md - Performance tuning
- doc/port_wamr.md - Platform porting guide

## Testing & Examples
- tests/unit/README.md - Unit test info
- samples/README.md - Example applications

## Component Structure
- core/iwasm/ - VMcore runtime libraries
- product-mini/ - iwasm executable
- wamr-compiler/ - wamrc AOT compiler
