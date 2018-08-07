/**
 * \file bn_mul.h
 *
 * \brief  Multi-precision integer library
 *
 *  Copyright (C) 2006-2010, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 *      Multiply source vector [s] with b, add result
 *       to destination vector [d] and set carry c.
 *
 *      Currently supports:
 *
 *         . IA-32 (386+)         . AMD64 / EM64T
 *         . IA-32 (SSE2)         . Motorola 68000
 *         . PowerPC, 32-bit      . MicroBlaze
 *         . PowerPC, 64-bit      . TriCore
 *         . SPARC v8             . ARM v3+
 *         . Alpha                . MIPS32
 *         . C, longlong          . C, generic
 */
#ifndef POLARSSL_BN_MUL_H
#define POLARSSL_BN_MUL_H

#include "bignum.h"

#if defined(POLARSSL_HAVE_ASM)

#if defined(__GNUC__)
#if defined(__i386__)

#define MULADDC_INIT                \
    __asm__( "                          \
        movl   %%ebx, %0;           \
        movl   %5, %%esi;           \
        movl   %6, %%edi;           \
        movl   %7, %%ecx;           \
        movl   %8, %%ebx;           \
        "

#define MULADDC_CORE                \
        "                           \
        lodsl;                      \
        mull   %%ebx;               \
        addl   %%ecx,   %%eax;      \
        adcl   $0,      %%edx;      \
        addl   (%%edi), %%eax;      \
        adcl   $0,      %%edx;      \
        movl   %%edx,   %%ecx;      \
        stosl;                      \
        "

#if defined(POLARSSL_HAVE_SSE2)

#define MULADDC_HUIT                    \
        "                               \
        movd     %%ecx,     %%mm1;      \
        movd     %%ebx,     %%mm0;      \
        movd     (%%edi),   %%mm3;      \
        paddq    %%mm3,     %%mm1;      \
        movd     (%%esi),   %%mm2;      \
        pmuludq  %%mm0,     %%mm2;      \
        movd     4(%%esi),  %%mm4;      \
        pmuludq  %%mm0,     %%mm4;      \
        movd     8(%%esi),  %%mm6;      \
        pmuludq  %%mm0,     %%mm6;      \
        movd     12(%%esi), %%mm7;      \
        pmuludq  %%mm0,     %%mm7;      \
        paddq    %%mm2,     %%mm1;      \
        movd     4(%%edi),  %%mm3;      \
        paddq    %%mm4,     %%mm3;      \
        movd     8(%%edi),  %%mm5;      \
        paddq    %%mm6,     %%mm5;      \
        movd     12(%%edi), %%mm4;      \
        paddq    %%mm4,     %%mm7;      \
        movd     %%mm1,     (%%edi);    \
        movd     16(%%esi), %%mm2;      \
        pmuludq  %%mm0,     %%mm2;      \
        psrlq    $32,       %%mm1;      \
        movd     20(%%esi), %%mm4;      \
        pmuludq  %%mm0,     %%mm4;      \
        paddq    %%mm3,     %%mm1;      \
        movd     24(%%esi), %%mm6;      \
        pmuludq  %%mm0,     %%mm6;      \
        movd     %%mm1,     4(%%edi);   \
        psrlq    $32,       %%mm1;      \
        movd     28(%%esi), %%mm3;      \
        pmuludq  %%mm0,     %%mm3;      \
        paddq    %%mm5,     %%mm1;      \
        movd     16(%%edi), %%mm5;      \
        paddq    %%mm5,     %%mm2;      \
        movd     %%mm1,     8(%%edi);   \
        psrlq    $32,       %%mm1;      \
        paddq    %%mm7,     %%mm1;      \
        movd     20(%%edi), %%mm5;      \
        paddq    %%mm5,     %%mm4;      \
        movd     %%mm1,     12(%%edi);  \
        psrlq    $32,       %%mm1;      \
        paddq    %%mm2,     %%mm1;      \
        movd     24(%%edi), %%mm5;      \
        paddq    %%mm5,     %%mm6;      \
        movd     %%mm1,     16(%%edi);  \
        psrlq    $32,       %%mm1;      \
        paddq    %%mm4,     %%mm1;      \
        movd     28(%%edi), %%mm5;      \
        paddq    %%mm5,     %%mm3;      \
        movd     %%mm1,     20(%%edi);  \
        psrlq    $32,       %%mm1;      \
        paddq    %%mm6,     %%mm1;      \
        movd     %%mm1,     24(%%edi);  \
        psrlq    $32,       %%mm1;      \
        paddq    %%mm3,     %%mm1;      \
        movd     %%mm1,     28(%%edi);  \
        addl     $32,       %%edi;      \
        addl     $32,       %%esi;      \
        psrlq    $32,       %%mm1;      \
        movd     %%mm1,     %%ecx;      \
        "

#define MULADDC_STOP            \
        "                       \
        emms;                   \
        movl   %4, %%ebx;       \
        movl   %%ecx, %1;       \
        movl   %%edi, %2;       \
        movl   %%esi, %3;       \
        "                       \
        : "=m" (t), "=m" (c), "=m" (d), "=m" (s)        \
        : "m" (t), "m" (s), "m" (d), "m" (c), "m" (b)   \
        : "eax", "ecx", "edx", "esi", "edi"             \
    );

#else

#define MULADDC_STOP            \
        "                       \
        movl   %4, %%ebx;       \
        movl   %%ecx, %1;       \
        movl   %%edi, %2;       \
        movl   %%esi, %3;       \
        "                       \
        : "=m" (t), "=m" (c), "=m" (d), "=m" (s)        \
        : "m" (t), "m" (s), "m" (d), "m" (c), "m" (b)   \
        : "eax", "ecx", "edx", "esi", "edi"             \
    );
#endif /* SSE2 */
#endif /* i386 */

#if defined(__amd64__) || defined (__x86_64__)

#define MULADDC_INIT                            \
    __asm__( "movq   %0, %%rsi      " :: "m" (s));  \
    __asm__( "movq   %0, %%rdi      " :: "m" (d));  \
    __asm__( "movq   %0, %%rcx      " :: "m" (c));  \
    __asm__( "movq   %0, %%rbx      " :: "m" (b));  \
    __asm__( "xorq   %r8, %r8       " );

#define MULADDC_CORE                            \
    __asm__( "movq  (%rsi),%rax     " );            \
    __asm__( "mulq   %rbx           " );            \
    __asm__( "addq   $8,   %rsi     " );            \
    __asm__( "addq   %rcx, %rax     " );            \
    __asm__( "movq   %r8,  %rcx     " );            \
    __asm__( "adcq   $0,   %rdx     " );            \
    __asm__( "nop                   " );            \
    __asm__( "addq   %rax, (%rdi)   " );            \
    __asm__( "adcq   %rdx, %rcx     " );            \
    __asm__( "addq   $8,   %rdi     " );

#define MULADDC_STOP                            \
    __asm__( "movq   %%rcx, %0      " : "=m" (c));  \
    __asm__( "movq   %%rdi, %0      " : "=m" (d));  \
    __asm__( "movq   %%rsi, %0      " : "=m" (s) :: \
    "rax", "rcx", "rdx", "rbx", "rsi", "rdi", "r8" );

#endif /* AMD64 */

#if defined(__mc68020__) || defined(__mcpu32__)

#define MULADDC_INIT                            \
    __asm__( "movl   %0, %%a2       " :: "m" (s));  \
    __asm__( "movl   %0, %%a3       " :: "m" (d));  \
    __asm__( "movl   %0, %%d3       " :: "m" (c));  \
    __asm__( "movl   %0, %%d2       " :: "m" (b));  \
    __asm__( "moveq  #0, %d0        " );

#define MULADDC_CORE                            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d4:%d1   " );            \
    __asm__( "addl   %d3, %d1       " );            \
    __asm__( "addxl  %d0, %d4       " );            \
    __asm__( "moveq  #0,  %d3       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "addxl  %d4, %d3       " );

#define MULADDC_STOP                            \
    __asm__( "movl   %%d3, %0       " : "=m" (c));  \
    __asm__( "movl   %%a3, %0       " : "=m" (d));  \
    __asm__( "movl   %%a2, %0       " : "=m" (s) :: \
    "d0", "d1", "d2", "d3", "d4", "a2", "a3" );

#define MULADDC_HUIT                            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d4:%d1   " );            \
    __asm__( "addxl  %d3, %d1       " );            \
    __asm__( "addxl  %d0, %d4       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d3:%d1   " );            \
    __asm__( "addxl  %d4, %d1       " );            \
    __asm__( "addxl  %d0, %d3       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d4:%d1   " );            \
    __asm__( "addxl  %d3, %d1       " );            \
    __asm__( "addxl  %d0, %d4       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d3:%d1   " );            \
    __asm__( "addxl  %d4, %d1       " );            \
    __asm__( "addxl  %d0, %d3       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d4:%d1   " );            \
    __asm__( "addxl  %d3, %d1       " );            \
    __asm__( "addxl  %d0, %d4       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d3:%d1   " );            \
    __asm__( "addxl  %d4, %d1       " );            \
    __asm__( "addxl  %d0, %d3       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d4:%d1   " );            \
    __asm__( "addxl  %d3, %d1       " );            \
    __asm__( "addxl  %d0, %d4       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "movel  %a2@+, %d1     " );            \
    __asm__( "mulul  %d2, %d3:%d1   " );            \
    __asm__( "addxl  %d4, %d1       " );            \
    __asm__( "addxl  %d0, %d3       " );            \
    __asm__( "addl   %d1, %a3@+     " );            \
    __asm__( "addxl  %d0, %d3       " );

#endif /* MC68000 */

#if defined(__powerpc__)   || defined(__ppc__)
#if defined(__powerpc64__) || defined(__ppc64__)

#if defined(__MACH__) && defined(__APPLE__)

#define MULADDC_INIT                            \
    __asm__( "ld     r3, %0         " :: "m" (s));  \
    __asm__( "ld     r4, %0         " :: "m" (d));  \
    __asm__( "ld     r5, %0         " :: "m" (c));  \
    __asm__( "ld     r6, %0         " :: "m" (b));  \
    __asm__( "addi   r3, r3, -8     " );            \
    __asm__( "addi   r4, r4, -8     " );            \
    __asm__( "addic  r5, r5,  0     " );

#define MULADDC_CORE                            \
    __asm__( "ldu    r7, 8(r3)      " );            \
    __asm__( "mulld  r8, r7, r6     " );            \
    __asm__( "mulhdu r9, r7, r6     " );            \
    __asm__( "adde   r8, r8, r5     " );            \
    __asm__( "ld     r7, 8(r4)      " );            \
    __asm__( "addze  r5, r9         " );            \
    __asm__( "addc   r8, r8, r7     " );            \
    __asm__( "stdu   r8, 8(r4)      " );

#define MULADDC_STOP                            \
    __asm__( "addze  r5, r5         " );            \
    __asm__( "addi   r4, r4, 8      " );            \
    __asm__( "addi   r3, r3, 8      " );            \
    __asm__( "std    r5, %0         " : "=m" (c));  \
    __asm__( "std    r4, %0         " : "=m" (d));  \
    __asm__( "std    r3, %0         " : "=m" (s) :: \
    "r3", "r4", "r5", "r6", "r7", "r8", "r9" );

#else

#define MULADDC_INIT                            \
    __asm__( "ld     %%r3, %0       " :: "m" (s));  \
    __asm__( "ld     %%r4, %0       " :: "m" (d));  \
    __asm__( "ld     %%r5, %0       " :: "m" (c));  \
    __asm__( "ld     %%r6, %0       " :: "m" (b));  \
    __asm__( "addi   %r3, %r3, -8   " );            \
    __asm__( "addi   %r4, %r4, -8   " );            \
    __asm__( "addic  %r5, %r5,  0   " );

#define MULADDC_CORE                            \
    __asm__( "ldu    %r7, 8(%r3)    " );            \
    __asm__( "mulld  %r8, %r7, %r6  " );            \
    __asm__( "mulhdu %r9, %r7, %r6  " );            \
    __asm__( "adde   %r8, %r8, %r5  " );            \
    __asm__( "ld     %r7, 8(%r4)    " );            \
    __asm__( "addze  %r5, %r9       " );            \
    __asm__( "addc   %r8, %r8, %r7  " );            \
    __asm__( "stdu   %r8, 8(%r4)    " );

#define MULADDC_STOP                            \
    __asm__( "addze  %r5, %r5       " );            \
    __asm__( "addi   %r4, %r4, 8    " );            \
    __asm__( "addi   %r3, %r3, 8    " );            \
    __asm__( "std    %%r5, %0       " : "=m" (c));  \
    __asm__( "std    %%r4, %0       " : "=m" (d));  \
    __asm__( "std    %%r3, %0       " : "=m" (s) :: \
    "r3", "r4", "r5", "r6", "r7", "r8", "r9" );

#endif

#else /* PPC32 */

#if defined(__MACH__) && defined(__APPLE__)

#define MULADDC_INIT                            \
    __asm__( "lwz    r3, %0         " :: "m" (s));  \
    __asm__( "lwz    r4, %0         " :: "m" (d));  \
    __asm__( "lwz    r5, %0         " :: "m" (c));  \
    __asm__( "lwz    r6, %0         " :: "m" (b));  \
    __asm__( "addi   r3, r3, -4     " );            \
    __asm__( "addi   r4, r4, -4     " );            \
    __asm__( "addic  r5, r5,  0     " );

#define MULADDC_CORE                            \
    __asm__( "lwzu   r7, 4(r3)      " );            \
    __asm__( "mullw  r8, r7, r6     " );            \
    __asm__( "mulhwu r9, r7, r6     " );            \
    __asm__( "adde   r8, r8, r5     " );            \
    __asm__( "lwz    r7, 4(r4)      " );            \
    __asm__( "addze  r5, r9         " );            \
    __asm__( "addc   r8, r8, r7     " );            \
    __asm__( "stwu   r8, 4(r4)      " );

#define MULADDC_STOP                            \
    __asm__( "addze  r5, r5         " );            \
    __asm__( "addi   r4, r4, 4      " );            \
    __asm__( "addi   r3, r3, 4      " );            \
    __asm__( "stw    r5, %0         " : "=m" (c));  \
    __asm__( "stw    r4, %0         " : "=m" (d));  \
    __asm__( "stw    r3, %0         " : "=m" (s) :: \
    "r3", "r4", "r5", "r6", "r7", "r8", "r9" );

#else

#define MULADDC_INIT                            \
    __asm__( "lwz    %%r3, %0       " :: "m" (s));  \
    __asm__( "lwz    %%r4, %0       " :: "m" (d));  \
    __asm__( "lwz    %%r5, %0       " :: "m" (c));  \
    __asm__( "lwz    %%r6, %0       " :: "m" (b));  \
    __asm__( "addi   %r3, %r3, -4   " );            \
    __asm__( "addi   %r4, %r4, -4   " );            \
    __asm__( "addic  %r5, %r5,  0   " );

#define MULADDC_CORE                            \
    __asm__( "lwzu   %r7, 4(%r3)    " );            \
    __asm__( "mullw  %r8, %r7, %r6  " );            \
    __asm__( "mulhwu %r9, %r7, %r6  " );            \
    __asm__( "adde   %r8, %r8, %r5  " );            \
    __asm__( "lwz    %r7, 4(%r4)    " );            \
    __asm__( "addze  %r5, %r9       " );            \
    __asm__( "addc   %r8, %r8, %r7  " );            \
    __asm__( "stwu   %r8, 4(%r4)    " );

#define MULADDC_STOP                            \
    __asm__( "addze  %r5, %r5       " );            \
    __asm__( "addi   %r4, %r4, 4    " );            \
    __asm__( "addi   %r3, %r3, 4    " );            \
    __asm__( "stw    %%r5, %0       " : "=m" (c));  \
    __asm__( "stw    %%r4, %0       " : "=m" (d));  \
    __asm__( "stw    %%r3, %0       " : "=m" (s) :: \
    "r3", "r4", "r5", "r6", "r7", "r8", "r9" );

#endif

#endif /* PPC32 */
#endif /* PPC64 */

#if defined(__sparc__) && defined(__sparc64__)

#define MULADDC_INIT                            \
    __asm__(                                        \
         "                                      \
                ldx     %3, %%o0;               \
                ldx     %4, %%o1;               \
                ld      %5, %%o2;               \
                ld      %6, %%o3;               \
         "

#define MULADDC_CORE                            \
         "                                      \
                ld      [%%o0], %%o4;           \
                inc     4, %%o0;                \
                ld      [%%o1], %%o5;           \
                umul    %%o3, %%o4, %%o4;       \
                addcc   %%o4, %%o2, %%o4;       \
                rd      %%y, %%g1;              \
                addx    %%g1, 0, %%g1;          \
                addcc   %%o4, %%o5, %%o4;       \
                st      %%o4, [%%o1];           \
                addx    %%g1, 0, %%o2;          \
                inc     4, %%o1;                \
        "

#define MULADDC_STOP                            \
        "                                       \
                st      %%o2, %0;               \
                stx     %%o1, %1;               \
                stx     %%o0, %2;               \
        "                                       \
        : "=m" (c), "=m" (d), "=m" (s)          \
        : "m" (s), "m" (d), "m" (c), "m" (b)    \
        : "g1", "o0", "o1", "o2", "o3", "o4",   \
          "o5"                                  \
        );
#endif /* SPARCv9 */

#if defined(__sparc__) && !defined(__sparc64__)

#define MULADDC_INIT                            \
    __asm__(                                        \
         "                                      \
                ld      %3, %%o0;               \
                ld      %4, %%o1;               \
                ld      %5, %%o2;               \
                ld      %6, %%o3;               \
         "

#define MULADDC_CORE                            \
         "                                      \
                ld      [%%o0], %%o4;           \
                inc     4, %%o0;                \
                ld      [%%o1], %%o5;           \
                umul    %%o3, %%o4, %%o4;       \
                addcc   %%o4, %%o2, %%o4;       \
                rd      %%y, %%g1;              \
                addx    %%g1, 0, %%g1;          \
                addcc   %%o4, %%o5, %%o4;       \
                st      %%o4, [%%o1];           \
                addx    %%g1, 0, %%o2;          \
                inc     4, %%o1;                \
        "

#define MULADDC_STOP                            \
        "                                       \
                st      %%o2, %0;               \
                st      %%o1, %1;               \
                st      %%o0, %2;               \
        "                                       \
        : "=m" (c), "=m" (d), "=m" (s)          \
        : "m" (s), "m" (d), "m" (c), "m" (b)    \
        : "g1", "o0", "o1", "o2", "o3", "o4",   \
          "o5"                                  \
        );

#endif /* SPARCv8 */

#if defined(__microblaze__) || defined(microblaze)

#define MULADDC_INIT                            \
    __asm__( "lwi   r3,   %0        " :: "m" (s));  \
    __asm__( "lwi   r4,   %0        " :: "m" (d));  \
    __asm__( "lwi   r5,   %0        " :: "m" (c));  \
    __asm__( "lwi   r6,   %0        " :: "m" (b));  \
    __asm__( "andi  r7,   r6, 0xffff" );            \
    __asm__( "bsrli r6,   r6, 16    " );

#define MULADDC_CORE                            \
    __asm__( "lhui  r8,   r3,   0   " );            \
    __asm__( "addi  r3,   r3,   2   " );            \
    __asm__( "lhui  r9,   r3,   0   " );            \
    __asm__( "addi  r3,   r3,   2   " );            \
    __asm__( "mul   r10,  r9,  r6   " );            \
    __asm__( "mul   r11,  r8,  r7   " );            \
    __asm__( "mul   r12,  r9,  r7   " );            \
    __asm__( "mul   r13,  r8,  r6   " );            \
    __asm__( "bsrli  r8, r10,  16   " );            \
    __asm__( "bsrli  r9, r11,  16   " );            \
    __asm__( "add   r13, r13,  r8   " );            \
    __asm__( "add   r13, r13,  r9   " );            \
    __asm__( "bslli r10, r10,  16   " );            \
    __asm__( "bslli r11, r11,  16   " );            \
    __asm__( "add   r12, r12, r10   " );            \
    __asm__( "addc  r13, r13,  r0   " );            \
    __asm__( "add   r12, r12, r11   " );            \
    __asm__( "addc  r13, r13,  r0   " );            \
    __asm__( "lwi   r10,  r4,   0   " );            \
    __asm__( "add   r12, r12, r10   " );            \
    __asm__( "addc  r13, r13,  r0   " );            \
    __asm__( "add   r12, r12,  r5   " );            \
    __asm__( "addc   r5, r13,  r0   " );            \
    __asm__( "swi   r12,  r4,   0   " );            \
    __asm__( "addi   r4,  r4,   4   " );

#define MULADDC_STOP                            \
    __asm__( "swi   r5,   %0        " : "=m" (c));  \
    __asm__( "swi   r4,   %0        " : "=m" (d));  \
    __asm__( "swi   r3,   %0        " : "=m" (s) :: \
     "r3", "r4" , "r5" , "r6" , "r7" , "r8" ,   \
     "r9", "r10", "r11", "r12", "r13" );

#endif /* MicroBlaze */

#if defined(__tricore__)

#define MULADDC_INIT                            \
    __asm__( "ld.a   %%a2, %0       " :: "m" (s));  \
    __asm__( "ld.a   %%a3, %0       " :: "m" (d));  \
    __asm__( "ld.w   %%d4, %0       " :: "m" (c));  \
    __asm__( "ld.w   %%d1, %0       " :: "m" (b));  \
    __asm__( "xor    %d5, %d5       " );

#define MULADDC_CORE                            \
    __asm__( "ld.w   %d0,   [%a2+]      " );        \
    __asm__( "madd.u %e2, %e4, %d0, %d1 " );        \
    __asm__( "ld.w   %d0,   [%a3]       " );        \
    __asm__( "addx   %d2,    %d2,  %d0  " );        \
    __asm__( "addc   %d3,    %d3,    0  " );        \
    __asm__( "mov    %d4,    %d3        " );        \
    __asm__( "st.w  [%a3+],  %d2        " );

#define MULADDC_STOP                            \
    __asm__( "st.w   %0, %%d4       " : "=m" (c));  \
    __asm__( "st.a   %0, %%a3       " : "=m" (d));  \
    __asm__( "st.a   %0, %%a2       " : "=m" (s) :: \
    "d0", "d1", "e2", "d4", "a2", "a3" );

#endif /* TriCore */

#if defined(__arm__)

#if defined(__thumb__) && !defined(__thumb2__)

#define MULADDC_INIT                            \
    __asm__(                                        \
         "                                      \
            ldr    r0, %3;                      \
            ldr    r1, %4;                      \
            ldr    r2, %5;                      \
            ldr    r3, %6;                      \
            lsr    r7, r3, #16;                 \
            mov    r9, r7;                      \
            lsl    r7, r3, #16;                 \
            lsr    r7, r7, #16;                 \
            mov    r8, r7;                      \
         "

#define MULADDC_CORE                            \
         "                                      \
            ldmia  r0!, {r6};                   \
            lsr    r7, r6, #16;                 \
            lsl    r6, r6, #16;                 \
            lsr    r6, r6, #16;                 \
            mov    r4, r8;                      \
            mul    r4, r6;                      \
            mov    r3, r9;                      \
            mul    r6, r3;                      \
            mov    r5, r9;                      \
            mul    r5, r7;                      \
            mov    r3, r8;                      \
            mul    r7, r3;                      \
            lsr    r3, r6, #16;                 \
            add    r5, r5, r3;                  \
            lsr    r3, r7, #16;                 \
            add    r5, r5, r3;                  \
            add    r4, r4, r2;                  \
            mov    r2, #0;                      \
            adc    r5, r2;                      \
            lsl    r3, r6, #16;                 \
            add    r4, r4, r3;                  \
            adc    r5, r2;                      \
            lsl    r3, r7, #16;                 \
            add    r4, r4, r3;                  \
            adc    r5, r2;                      \
            ldr    r3, [r1];                    \
            add    r4, r4, r3;                  \
            adc    r2, r5;                      \
            stmia  r1!, {r4};                   \
         "

#define MULADDC_STOP                            \
         "                                      \
            str    r2, %0;                      \
            str    r1, %1;                      \
            str    r0, %2;                      \
         "                                      \
         : "=m" (c),  "=m" (d), "=m" (s)        \
         : "m" (s), "m" (d), "m" (c), "m" (b)   \
         : "r0", "r1", "r2", "r3", "r4", "r5",  \
           "r6", "r7", "r8", "r9", "cc"         \
         );

#else

#define MULADDC_INIT                            \
    __asm__(                                        \
         "                                     \
            ldr    r0, %3;                      \
            ldr    r1, %4;                      \
            ldr    r2, %5;                      \
            ldr    r3, %6;                      \
         "

#define MULADDC_CORE                            \
         "                                      \
            ldr    r4, [r0], #4;                \
            mov    r5, #0;                      \
            ldr    r6, [r1];                    \
            umlal  r2, r5, r3, r4;              \
            adds   r7, r6, r2;                  \
            adc    r2, r5, #0;                  \
            str    r7, [r1], #4;                \
         "

#define MULADDC_STOP                            \
         "                                      \
            str    r2, %0;                      \
            str    r1, %1;                      \
            str    r0, %2;                      \
         "                                      \
         : "=m" (c),  "=m" (d), "=m" (s)        \
         : "m" (s), "m" (d), "m" (c), "m" (b)   \
         : "r0", "r1", "r2", "r3", "r4", "r5",  \
           "r6", "r7", "cc"                     \
         );

#endif /* Thumb */

#endif /* ARMv3 */

#if defined(__alpha__)

#define MULADDC_INIT                            \
    __asm__( "ldq    $1, %0         " :: "m" (s));  \
    __asm__( "ldq    $2, %0         " :: "m" (d));  \
    __asm__( "ldq    $3, %0         " :: "m" (c));  \
    __asm__( "ldq    $4, %0         " :: "m" (b));

#define MULADDC_CORE                            \
    __asm__( "ldq    $6,  0($1)     " );            \
    __asm__( "addq   $1,  8, $1     " );            \
    __asm__( "mulq   $6, $4, $7     " );            \
    __asm__( "umulh  $6, $4, $6     " );            \
    __asm__( "addq   $7, $3, $7     " );            \
    __asm__( "cmpult $7, $3, $3     " );            \
    __asm__( "ldq    $5,  0($2)     " );            \
    __asm__( "addq   $7, $5, $7     " );            \
    __asm__( "cmpult $7, $5, $5     " );            \
    __asm__( "stq    $7,  0($2)     " );            \
    __asm__( "addq   $2,  8, $2     " );            \
    __asm__( "addq   $6, $3, $3     " );            \
    __asm__( "addq   $5, $3, $3     " );

#define MULADDC_STOP                            \
    __asm__( "stq    $3, %0         " : "=m" (c));  \
    __asm__( "stq    $2, %0         " : "=m" (d));  \
    __asm__( "stq    $1, %0         " : "=m" (s) :: \
    "$1", "$2", "$3", "$4", "$5", "$6", "$7" );

#endif /* Alpha */

#if defined(__mips__)

#define MULADDC_INIT                            \
    __asm__( "lw     $10, %0        " :: "m" (s));  \
    __asm__( "lw     $11, %0        " :: "m" (d));  \
    __asm__( "lw     $12, %0        " :: "m" (c));  \
    __asm__( "lw     $13, %0        " :: "m" (b));

#define MULADDC_CORE                            \
    __asm__( "lw     $14, 0($10)    " );            \
    __asm__( "multu  $13, $14       " );            \
    __asm__( "addi   $10, $10, 4    " );            \
    __asm__( "mflo   $14            " );            \
    __asm__( "mfhi   $9             " );            \
    __asm__( "addu   $14, $12, $14  " );            \
    __asm__( "lw     $15, 0($11)    " );            \
    __asm__( "sltu   $12, $14, $12  " );            \
    __asm__( "addu   $15, $14, $15  " );            \
    __asm__( "sltu   $14, $15, $14  " );            \
    __asm__( "addu   $12, $12, $9   " );            \
    __asm__( "sw     $15, 0($11)    " );            \
    __asm__( "addu   $12, $12, $14  " );            \
    __asm__( "addi   $11, $11, 4    " );

#define MULADDC_STOP                            \
    __asm__( "sw     $12, %0        " : "=m" (c));  \
    __asm__( "sw     $11, %0        " : "=m" (d));  \
    __asm__( "sw     $10, %0        " : "=m" (s) :: \
    "$9", "$10", "$11", "$12", "$13", "$14", "$15" );

#endif /* MIPS */
#endif /* GNUC */

#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)

#define MULADDC_INIT                            \
    ____asm__   mov     esi, s                      \
    ____asm__   mov     edi, d                      \
    ____asm__   mov     ecx, c                      \
    ____asm__   mov     ebx, b

#define MULADDC_CORE                            \
    ____asm__   lodsd                               \
    ____asm__   mul     ebx                         \
    ____asm__   add     eax, ecx                    \
    ____asm__   adc     edx, 0                      \
    ____asm__   add     eax, [edi]                  \
    ____asm__   adc     edx, 0                      \
    ____asm__   mov     ecx, edx                    \
    ____asm__   stosd

#if defined(POLARSSL_HAVE_SSE2)

#define EMIT ____asm__ _emit

#define MULADDC_HUIT                            \
    EMIT 0x0F  EMIT 0x6E  EMIT 0xC9             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0xC3             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x1F             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCB             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x16             \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xD0             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x66  EMIT 0x04  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xE0             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x76  EMIT 0x08  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xF0             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x7E  EMIT 0x0C  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xF8             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCA             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x5F  EMIT 0x04  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xDC             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x08  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xEE             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x67  EMIT 0x0C  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xFC             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x0F             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x56  EMIT 0x10  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xD0             \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x66  EMIT 0x14  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xE0             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCB             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x76  EMIT 0x18  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xF0             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x04  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x5E  EMIT 0x1C  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xD8             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCD             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x10  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xD5             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x08  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCF             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x14  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xE5             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x0C  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCA             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x18  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xF5             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x10  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCC             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x1C  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xDD             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x14  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCE             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x18  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCB             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x1C  \
    EMIT 0x83  EMIT 0xC7  EMIT 0x20             \
    EMIT 0x83  EMIT 0xC6  EMIT 0x20             \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0x7E  EMIT 0xC9

#define MULADDC_STOP                            \
    EMIT 0x0F  EMIT 0x77                        \
    ____asm__   mov     c, ecx                      \
    ____asm__   mov     d, edi                      \
    ____asm__   mov     s, esi                      \

#else

#define MULADDC_STOP                            \
    ____asm__   mov     c, ecx                      \
    ____asm__   mov     d, edi                      \
    ____asm__   mov     s, esi                      \

#endif /* SSE2 */
#endif /* MSVC */

#endif /* POLARSSL_HAVE_ASM */

#if !defined(MULADDC_CORE)
#if defined(POLARSSL_HAVE_UDBL)

#define MULADDC_INIT                    \
{                                       \
    t_udbl r;                           \
    t_uint r0, r1;

#define MULADDC_CORE                    \
    r   = *(s++) * (t_udbl) b;          \
    r0  = r;                            \
    r1  = r >> biL;                     \
    r0 += c;  r1 += (r0 <  c);          \
    r0 += *d; r1 += (r0 < *d);          \
    c = r1; *(d++) = r0;

#define MULADDC_STOP                    \
}

#else
#define MULADDC_INIT                    \
{                                       \
    t_uint s0, s1, b0, b1;              \
    t_uint r0, r1, rx, ry;              \
    b0 = ( b << biH ) >> biH;           \
    b1 = ( b >> biH );

#define MULADDC_CORE                    \
    s0 = ( *s << biH ) >> biH;          \
    s1 = ( *s >> biH ); s++;            \
    rx = s0 * b1; r0 = s0 * b0;         \
    ry = s1 * b0; r1 = s1 * b1;         \
    r1 += ( rx >> biH );                \
    r1 += ( ry >> biH );                \
    rx <<= biH; ry <<= biH;             \
    r0 += rx; r1 += (r0 < rx);          \
    r0 += ry; r1 += (r0 < ry);          \
    r0 +=  c; r1 += (r0 <  c);          \
    r0 += *d; r1 += (r0 < *d);          \
    c = r1; *(d++) = r0;

#define MULADDC_STOP                    \
}

#endif /* C (generic)  */
#endif /* C (longlong) */

#endif /* bn_mul.h */
