#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/base_include.h"
#include "os/os_include.h"
#include "diagnostics/diagnostics_include.h"
#include "tokenizer/tokenizer_include.h"
#include "parser/parser_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "diagnostics/diagnostics_include.c"
#include "tokenizer/tokenizer_include.c"
#include "parser/parser_include.c"

s32
main(void)
{
	tk_tests();
	p_tests();
}
