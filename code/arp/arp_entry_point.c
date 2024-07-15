#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#include "base/base_include.h"
#include "os/os_include.h"
#include "tokenizer/tokenizer_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "tokenizer/tokenizer_include.c"

s32
main(void)
{
	Arena *arena = arena_alloc();

	String source = str_lit("hello world");
	TK_TokenizeResult result = {0};
	tk_tokenize(arena, &result, source);

	printf("tokenized %td tokens.\n", result.token_count);
	for (isize i = 0; i < result.token_count; i++) {
		TK_TokenKind kind = result.kinds[i];
		TK_Span span = result.spans[i];
		printf("%.*s@%d..%d\n", str_fmt(tk_string_from_token_kind(kind)), span.start,
		        span.end);
	}

	printf("found %td errors.\n", result.error_count);
	for (TK_Error *error = result.first_error; error != 0; error = error->next) {
		printf("error at %d..%d: %.*s\n", error->span.start, error->span.end,
		        str_fmt(error->message));
	}

	arena_release(arena);
}
