/*
 * Force-included into every XNU translation unit when building with stock
 * (non-Apple) clang. Apple's clang has builtins that describe the pointer/data
 * layout of a type for kalloc_type; upstream clang lacks them, so every type
 * is described as pure data (which is what the kernel's data-only assertions
 * ask for; claims of pointerfulness would fail them).
 */
#ifndef XNU_CLANG_COMPAT_H
#define XNU_CLANG_COMPAT_H
#if !defined(ASSEMBLER) && !defined(__ASSEMBLER__)
#if !__has_builtin(__builtin_xnu_type_signature)
#define XNU_NO_TYPE_BUILTINS                     1
#define __builtin_xnu_type_signature(type)       "12"
#define __builtin_xnu_type_summary(type)         ((1U << KT_GRANULE_PADDING) | (1U << KT_GRANULE_DATA))
#ifdef __cplusplus
#define __builtin_xnu_types_compatible(a, b)     __is_same(a, b)
#else
#define __builtin_xnu_types_compatible(a, b)     __builtin_types_compatible_p(a, b)
#endif
#endif
/* C23's counting builtins are Clang 19; before that, spell them out. */
#if !__has_builtin(__builtin_clzg)
#define __builtin_clzg(x, zero)                  ((x) ? __builtin_clzll((x)) : (zero))
#endif
#if !__has_builtin(__builtin_ctzg)
#define __builtin_ctzg(x, zero)                  ((x) ? __builtin_ctzll((x)) : (zero))
#endif
#endif
#endif
