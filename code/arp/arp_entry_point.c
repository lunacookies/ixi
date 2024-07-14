#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

s32
main(void)
{
	Arena *arena = arena_alloc();
	printf("%p\n", (void *)arena);

	isize *my_isize = push_struct(arena, isize);
	printf("allocated %p\n", (void *)my_isize);
	arena_clear(arena);

	my_isize = push_struct(arena, isize);
	printf("allocated %p\n", (void *)my_isize);

	arena_release(arena);
}
