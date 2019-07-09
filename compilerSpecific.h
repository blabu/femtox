/*
 * gccSpecific.h
 *
 *  Created on: 25 мар. 2019 г.
 *      Author: blabu
 */

#ifndef COMPILERSPECIFIC_H_
#define COMPILERSPECIFIC_H_

/**
 * @name    Compiler abstraction macros
 * @{
 */

#ifdef __GNUC__
/**
 * @brief   Allocates a variable or function to a specific section.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_SECTION(s)   __attribute__((section(s)))

/**
 * @brief   Marks a function or variable as a weak symbol.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_WEAK         __attribute__((weak))

/**
 * @brief   Marks a function or variable as used.
 * @details The compiler or linker shall not remove the marked function or
 *          variable regardless if it is referred or not in the code.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_USED         __attribute__((used))

/**
 * @brief   Enforces alignment of the variable or function declared afterward.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_ALIGN(n)     __attribute__((aligned(n)))

/**
 * @brief   Enforces packing of the structure declared afterward.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_PACK         __attribute__((packed))

/**
 * @brief   Marks a function as not inlineable.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_NO_INLINE    __attribute__((noinline))

/**
 * @brief   Enforces a function inline.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_FORCE_INLINE __attribute__((always_inline))

/**
 * @brief   Marks a function as non-returning.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_NO_RETURN    __attribute__((noreturn))

#endif //__GNUC__

#ifdef GHSSPECIFIC
/**
 * @brief   Allocates a variable or function to a specific section.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_SECTION(s)   __attribute__((section(s)))

/**
 * @brief   Marks a function or variable as a weak symbol.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_WEAK         __attribute__((weak))

/**
 * @brief   Marks a function or variable as used.
 * @details The compiler or linker shall not remove the marked function or
 *          variable regardless if it is referred or not in the code.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_USED         __attribute__((used))

/**
 * @brief   Enforces alignment of the variable or function declared afterward.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_ALIGN(n)     __attribute__((aligned(n)))

/**
 * @brief   Enforces packing of the structure declared afterward.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_PACK         __attribute__((packed))

/**
 * @brief   Marks a function as not inlineable.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_NO_INLINE    __noinline

/**
 * @brief   Enforces a function inline.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_FORCE_INLINE

/**
 * @brief   Marks a function as non-returning.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_NO_RETURN    __attribute__((noreturn))

#endif /* GHSSPECIFIC */


#ifdef CWSPECIFIC

#define CC_SECTION(s)   __declspec (section s)

/**
 * @brief   Marks a function or variable as a weak symbol.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_WEAK

/**
 * @brief   Marks a function or variable as used.
 * @details The compiler or linker shall not remove the marked function or
 *          variable regardless if it is referred or not in the code.
 * @note    If the compiler does not support such a feature then this macro
 *          must not be defined or it could originate errors.
 */
#define CC_USED         __attribute__((used))

#define CC_ALIGN(n)
#define CC_PACK

/**
 * @brief   Marks a function as not inlineable.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_NO_INLINE    __declspec(never_inline)

/**
 * @brief   Enforces a function inline.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_FORCE_INLINE //__attribute__((always_inline))

/**
 * @brief   Marks a function as non-returning.
 * @note    Can be implemented as an empty macro if not supported by the
 *          compiler.
 */
#define CC_NO_RETURN    //__attribute__((noreturn))

#endif /* CWSPECIFIC*/

#ifndef CC_NO_RETURN
#define CC_NO_RETURN
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


#endif /* COMPILERSPECIFIC_H_ */
