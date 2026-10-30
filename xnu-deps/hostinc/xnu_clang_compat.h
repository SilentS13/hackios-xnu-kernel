/*
 * Force-included into every XNU translation unit when building with stock
 * (non-Apple) clang. Apple's clang has builtins that describe the pointer/data
 * layout of a type for kalloc_type; upstream clang lacks them, so the summary
 * is stubbed by language. In C it has to read as a mixed layout: a data-only
 * summary makes kalloc_type_view_parse() skip every __kalloc_type view, and
 * kalloc_init() asserts on the empty table. In C++ it has to read as pure data,
 * because IOKit's data-only allocation templates (cxx_safe_buffers, OSData)
 * only exist for types the summary calls data.
 */
#ifndef XNU_CLANG_COMPAT_H
#define XNU_CLANG_COMPAT_H
#if !defined(ASSEMBLER) && !defined(__ASSEMBLER__)
#if !__has_builtin(__builtin_xnu_type_signature)
#define XNU_NO_TYPE_BUILTINS                     1
#define __builtin_xnu_type_signature(type)       "12"
#ifdef __cplusplus
#define __builtin_xnu_type_summary(type)         ((1U << KT_GRANULE_PADDING) | (1U << KT_GRANULE_DATA))
#define __builtin_xnu_types_compatible(a, b)     __is_same(a, b)
#else
/* Every granule: neither data-only, pointer-array nor overlapping. */
#define __builtin_xnu_type_summary(type)         ((1U << KT_GRANULE_PADDING) |  \
	(1U << KT_GRANULE_POINTER) | (1U << KT_GRANULE_DATA) |                    \
	(1U << KT_GRANULE_DUAL) | (1U << KT_GRANULE_PAC))
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
