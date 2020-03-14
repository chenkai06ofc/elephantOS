#ifndef __LIB_COMMON_H
#define __LIB_COMMON_H

#define NULL ((void *)0)

#define bool    int
#define true    1
#define false   0

#define field_to_struct_ptr(struct_type_name, field_name, field_ptr) \
    (struct_type_name*) ((uint32_t)field_ptr - (uint32_t)(&((struct_type_name*)0)->field_name))

typedef void* func_addr;
#endif //__LIB_COMMON_H
