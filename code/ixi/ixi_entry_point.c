#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <spawn.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/base_include.h"
#include "os/os_include.h"
#include "diagnostics/diagnostics_include.h"
#include "tokenizer/tokenizer_include.h"
#include "parser/parser_include.h"
#include "sema/sema_include.h"
#include "codegen/codegen_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "diagnostics/diagnostics_include.c"
#include "tokenizer/tokenizer_include.c"
#include "parser/parser_include.c"
#include "sema/sema_include.c"
#include "codegen/codegen_include.c"

s32
main(void)
{
	tk_tests();
	p_tests();
	sm_tests();
	cg_tests();
}
