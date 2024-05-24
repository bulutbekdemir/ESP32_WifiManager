/*!
* @file wm_generalMacros.h
*
*	@date 2024
* @author Bulut Bekdemir
* @brief General macros for struct initialization, deinitialization, retain and release functions.
* @note This functions meant to be used with struct types that are meant to be used with reference counting.
* @note If CONFIG_USE_REF_COUNT is defined, reference counting will be used, otherwise it will be ignored and 
*       only init and deinit functions will be generated. 
* 
*     Example usage:
*     struct MyStruct {
*         REF_COUNT_FIELD
*         int a;
*         int b;
*     };
*     INIT_FUNC(MyStruct)
*     DEINIT_FUNC(MyStruct)
*     RETAIN_FUNC(MyStruct)
*     RELEASE_FUNC(MyStruct)
*
*    MyStruct *ptr = MyStruct_init();
*    MyStruct_retain(ptr);
*    MyStruct_release(ptr);
*    MyStruct_deinit(ptr);
*
* @copyright BSD 3-Clause License
* @version 1.1.2+1
*/

#ifndef GENERALMACROS_H
#define GENERALMACROS_H

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#if defined(CONFIG_USE_REF_COUNT) 

#define HAS_REF_COUNT_FIELD(struct_type) \
    _Static_assert(!(offsetof(struct_type, ref_count) == 0), "Struct does not contain ref_count field");

#define DEFINE_INIT_FUNC(struct_type) \
    HAS_REF_COUNT_FIELD(struct_type) \
    struct_type *struct_type##_init() { \
        struct_type *ptr = (struct_type *)pvPortMalloc(sizeof(struct_type)); \
        if (ptr != NULL) { \
            memset(ptr, 0, sizeof(struct_type)); \
            ptr->ref_count = 1; \
        } \
        return ptr; \
    }

#define DEFINE_DEINIT_FUNC(struct_type) \
    HAS_REF_COUNT_FIELD(struct_type) \
    void struct_type##_deinit(struct_type *ptr) { \
        if (ptr != NULL) { \
            if (--ptr->ref_count == 0) { \
                vPortFree(ptr); \
            } \
        } \
    }

#define DEFINE_RETAIN_FUNC(struct_type) \
    HAS_REF_COUNT_FIELD(struct_type) \
    void struct_type##_retain(struct_type *ptr) { \
        if (ptr != NULL) { \
            ptr->ref_count++; \
        } \
    }

#define DEFINE_RELEASE_FUNC(struct_type) \
    HAS_REF_COUNT_FIELD(struct_type) \
    void struct_type##_release(struct_type *ptr) { \
        if (ptr != NULL) { \
            if (--ptr->ref_count == 0) { \
                vPortFree(ptr); \
            } \
        } \
    }

#define INIT_FUNC(struct_type) DEFINE_INIT_FUNC(struct_type)
#define DEINIT_FUNC(struct_type) DEFINE_DEINIT_FUNC(struct_type)
#define RETAIN_FUNC(struct_type) DEFINE_RETAIN_FUNC(struct_type)
#define RELEASE_FUNC(struct_type) DEFINE_RELEASE_FUNC(struct_type)

#define DECLARE_INIT_FUNC(struct_type) extern struct_type *struct_type##_init()
#define DECLARE_DEINIT_FUNC(struct_type) extern void struct_type##_deinit(struct_type *ptr)
#define DECLARE_RETAIN_FUNC(struct_type) extern void struct_type##_retain(struct_type *ptr)
#define DECLARE_RELEASE_FUNC(struct_type) extern void struct_type##_release(struct_type *ptr)

#else

#define INIT_FUNC(struct_type) \
    struct_type *struct_type##_init() { \
        struct_type *ptr = (struct_type *)pvPortMalloc(sizeof(struct_type)); \
        if (ptr != NULL) { \
            memset(ptr, 0, sizeof(struct_type)); \
        } \
        return ptr; \
    }

#define DEINIT_FUNC(struct_type) \
    void struct_type##_deinit(struct_type *ptr) { \
        if (ptr != NULL) { \
            vPortFree(ptr); \
        } \
    }

#define RETAIN_FUNC(struct_type) \
    _Static_assert(0, "RETAIN_FUNC is not supported for this struct type")

#define RELEASE_FUNC(struct_type) \
    _Static_assert(0, "RELEASE_FUNC is not supported for this struct type") 

#endif /* CONFIG_USE_REF_COUNT */

#endif /* GENERALMACROS_H */

