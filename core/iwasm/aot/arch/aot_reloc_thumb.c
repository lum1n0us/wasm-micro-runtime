/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "aot_reloc.h"

#define R_ARM_THM_CALL  10  /* PC relative (Thumb BL and ARMv5 Thumb BLX). */
#define R_ARM_THM_JMP24 30  /* B.W */

#ifndef BH_MB
#define BH_MB 1024 * 1024
#endif

void __divdi3();
void __udivdi3();
void __moddi3();
void __umoddi3();
void __divsi3();
void __udivsi3();
void __modsi3();
void __udivmoddi4();
void __clzsi2();
void __fixsfdi();
void __fixunssfdi();
void __fixdfdi();
void __fixunsdfdi();
void __floatdisf();
void __floatundisf();
void __floatdidf();
void __floatundidf();
void __aeabi_l2f();
void __aeabi_f2lz();
void __aeabi_ul2f();
void __aeabi_d2lz();
void __aeabi_l2d();
void __aeabi_f2ulz();
void __aeabi_ul2d();
void __aeabi_d2ulz();
void __aeabi_idiv();
void __aeabi_uidiv();
void __aeabi_idivmod();
void __aeabi_uidivmod();
void __aeabi_ldivmod();
void __aeabi_uldivmod();

static SymbolMap target_sym_map[] = {
    REG_COMMON_SYMBOLS,
    /* compiler-rt symbols that come from compiler(e.g. gcc) */
    REG_SYM(__divdi3),
    REG_SYM(__udivdi3),
    REG_SYM(__umoddi3),
    REG_SYM(__divsi3),
    REG_SYM(__udivsi3),
    REG_SYM(__modsi3),
    REG_SYM(__udivmoddi4),
    REG_SYM(__clzsi2),
    REG_SYM(__fixsfdi),
    REG_SYM(__fixunssfdi),
    REG_SYM(__fixdfdi),
    REG_SYM(__fixunsdfdi),
    REG_SYM(__floatdisf),
    REG_SYM(__floatundisf),
    REG_SYM(__floatdidf),
    REG_SYM(__floatundidf),
    REG_SYM(__aeabi_l2f),
    REG_SYM(__aeabi_f2lz),
    REG_SYM(__aeabi_ul2f),
    REG_SYM(__aeabi_d2lz),
    REG_SYM(__aeabi_l2d),
    REG_SYM(__aeabi_f2ulz),
    REG_SYM(__aeabi_ul2d),
    REG_SYM(__aeabi_d2ulz),
    REG_SYM(__aeabi_idiv),
    REG_SYM(__aeabi_uidiv),
    REG_SYM(__aeabi_idivmod),
    REG_SYM(__aeabi_uidivmod),
    REG_SYM(__aeabi_ldivmod),
    REG_SYM(__aeabi_uldivmod)
};

static void
set_error_buf(char *error_buf, uint32 error_buf_size, const char *string)
{
    if (error_buf != NULL)
        snprintf(error_buf, error_buf_size, "%s", string);
}

SymbolMap *
get_target_symbol_map(uint32 *sym_num)
{
    *sym_num = sizeof(target_sym_map) / sizeof(SymbolMap);
    return target_sym_map;
}

void
get_current_target(char *target_buf, uint32 target_buf_size)
{
    char *build_target = BUILD_TARGET;
    char *p = target_buf, *p_end;
    snprintf(target_buf, target_buf_size, "%s", build_target);
    p_end = p + strlen(target_buf);
    while (p < p_end) {
        if (*p >= 'A' && *p <= 'Z')
            *p++ += 'a' - 'A';
        else
            p++;
    }
    if (!strcmp(target_buf, "thumb"))
        snprintf(target_buf, target_buf_size, "thumbv4t");
}

uint32
get_plt_item_size()
{
    /* 16 bytes instructions and 4 bytes symbol address */
    return 20;
}

uint32
get_plt_table_size()
{
    return get_plt_item_size() * (sizeof(target_sym_map) / sizeof(SymbolMap));
}

void
init_plt_table(uint8 *plt)
{
    uint32 i, num = sizeof(target_sym_map) / sizeof(SymbolMap);
    for (i = 0; i < num; i++) {
        uint16 *p = (uint16*)plt;
        /* push {lr} */
        *p++ = 0xb500;
        /* push {r4, r5} */
        *p++ = 0xb430;
        /* add  r4, pc, #8 */
        *p++ = 0xa402;
        /* ldr  r5, [r4, #0] */
        *p++ = 0x6825;
        /* blx  r5 */
        *p++ = 0x47a8;
        /* pop  {r4, r5} */
        *p++ = 0xbc30;
        /* pop  {pc} */
        *p++ = 0xbd00;
        p++;
        /* symbol addr */
        *(uint32*)p = (uint32)(uintptr_t)target_sym_map[i].symbol_addr;;
        plt += get_plt_item_size();
    }
}

static bool
check_reloc_offset(uint32 target_section_size,
                   uint64 reloc_offset, uint32 reloc_data_size,
                   char *error_buf, uint32 error_buf_size)
{
    if (!(reloc_offset < (uint64)target_section_size
          && reloc_offset + reloc_data_size <= (uint64)target_section_size)) {
        set_error_buf(error_buf, error_buf_size,
                      "AOT module load failed: invalid relocation offset.");
        return false;
    }
    return true;
}

bool
apply_relocation(AOTModule *module,
                 uint8 *target_section_addr, uint32 target_section_size,
                 uint64 reloc_offset, uint64 reloc_addend,
                 uint32 reloc_type, void *symbol_addr, int32 symbol_index,
                 char *error_buf, uint32 error_buf_size)
{
    switch (reloc_type) {
        /* TODO: implement THUMB relocation */
        case R_ARM_THM_CALL:
        case R_ARM_THM_JMP24:

        default:
            if (error_buf != NULL)
                snprintf(error_buf, error_buf_size,
                         "Load relocation section failed: "
                         "invalid relocation type %d.",
                         reloc_type);
            return false;
    }
    return true;
}

