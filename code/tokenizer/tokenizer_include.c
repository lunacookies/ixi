enum {
	tk_token_chunk_capacity = 1024,
};

typedef struct TK_Chunk TK_Chunk;
struct TK_Chunk {
	TK_Chunk *next;
	TK_Chunk *prev;

	isize token_count;
	TK_TokenKind kinds[tk_token_chunk_capacity];
	TK_Span spans[tk_token_chunk_capacity];
};

typedef struct TK_Tokenizer TK_Tokenizer;
struct TK_Tokenizer {
	String source;

	isize token_count;
	TK_Chunk *first_chunk;
	TK_Chunk *last_chunk;

	TK_Error *first_error;
	TK_Error *last_error;
	isize error_count;
};

function void
tk_emit(Arena *temp_arena, TK_Tokenizer *tokenizer, TK_TokenKind kind, isize start, isize end)
{
	TK_Chunk *chunk = 0;

	if (tokenizer->first_chunk == 0) {
		assert(tokenizer->last_chunk == 0);
		chunk = push_struct(temp_arena, TK_Chunk);
		tokenizer->first_chunk = chunk;
		tokenizer->last_chunk = chunk;
	} else {
		assert(tokenizer->last_chunk != 0);
		assert(tokenizer->last_chunk->token_count <= tk_token_chunk_capacity);
		if (tokenizer->last_chunk->token_count == tk_token_chunk_capacity) {
			chunk = push_struct(temp_arena, TK_Chunk);
			chunk->prev = tokenizer->last_chunk;
			tokenizer->last_chunk->next = chunk;
			tokenizer->last_chunk = chunk;
		} else {
			chunk = tokenizer->last_chunk;
		}
	}

	TK_Span span = {(s32)start, (s32)end};
	chunk->kinds[chunk->token_count] = kind;
	chunk->spans[chunk->token_count] = span;
	chunk->token_count++;
	tokenizer->token_count++;
}

function void
tk_error(Arena *arena, TK_Tokenizer *tokenizer, TK_Span span, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	tokenizer->error_count++;

	TK_Error *error = push_struct(arena, TK_Error);
	error->span = span;
	error->message = push_stringfv(arena, fmt, ap);

	if (tokenizer->first_error == 0) {
		assert(tokenizer->last_error == 0);
		tokenizer->first_error = error;
		tokenizer->last_error = error;
	} else {
		assert(tokenizer->last_error != 0);
		error->prev = tokenizer->last_error;
		tokenizer->last_error->next = error;
		tokenizer->last_error = error;
	}

	va_end(ap);
}

function void
tk_tokenize(Arena *arena, TK_TokenizeResult *result, String source)
{
	memory_zero_struct(result);

	TK_Tokenizer tokenizer = {0};
	tokenizer.source = source;

	Temp temp = temp_begin(&arena, 1);

	for (isize i = 0; i < source.length; i++) {
		if (source.data[i] == ' ' || source.data[i] == '\t' || source.data[i] == '\n') {
			continue;
		}

		tk_emit(temp.arena, &tokenizer, TK_TokenKind_Identifier, i, i + 1);
		TK_Span span = {(s32)i, (s32)(i + 1)};
		tk_error(arena, &tokenizer, span, "died at %td", i);
	}

	TK_TokenKind *kinds = push_array(arena, TK_TokenKind, tokenizer.token_count);
	TK_Span *spans = push_array(arena, TK_Span, tokenizer.token_count);

	isize i = 0;
	for (TK_Chunk *chunk = tokenizer.first_chunk; chunk != 0; chunk = chunk->next) {
		memory_copy_array(kinds + i, chunk->kinds, chunk->token_count);
		memory_copy_array(spans + i, chunk->spans, chunk->token_count);
		i += chunk->token_count;
	}

	temp_end(temp);

	result->token_count = tokenizer.token_count;
	result->kinds = kinds;
	result->spans = spans;

	result->error_count = tokenizer.error_count;
	result->first_error = tokenizer.first_error;
	result->last_error = tokenizer.last_error;
}

function String
tk_string_from_token_kind(TK_TokenKind kind)
{
	String result = {0};

	switch (kind) {
		case TK_TokenKind_Error: result = str_lit("Error"); break;
		case TK_TokenKind_Identifier: result = str_lit("Identifier"); break;
	}

	return result;
}

function void
tk_tokenize_result_stringify(Arena *arena, TK_TokenizeResult tokenize, StringList *list)
{
	string_list_pushf(arena, list, "%td tokens:\n", tokenize.token_count);
	for (isize i = 0; i < tokenize.token_count; i++) {
		TK_TokenKind kind = tokenize.kinds[i];
		TK_Span span = tokenize.spans[i];

		string_list_push(arena, list, tk_string_from_token_kind(kind));
		string_list_pushf(arena, list, "@%d..%d\n", span.start, span.end);
	}

	string_list_pushf(arena, list, "%td errors:\n", tokenize.error_count);
	for (TK_Error *error = tokenize.first_error; error != 0; error = error->next) {
		string_list_pushf(arena, list, "error at %d..%d: %.*s\n", error->span.start,
		        error->span.end, str_fmt(error->message));
	}
}

function void
tk_tests(void)
{
	Temp temp = temp_begin(0, 0);

	OS_Entry *entries = os_directory_entries(temp.arena, str_lit("code/tokenizer/test_data"));

	for (OS_Entry *entry = entries; entry != 0; entry = entry->next) {
		if (entry->is_directory) {
			continue;
		}

		String contents = os_read_file(temp.arena, entry->path);

		String source = {0};
		String expected_output = {0};
		b32 found = string_cut(contents, &source, &expected_output, str_lit("===\n"));
		assert(found);

		TK_TokenizeResult tokenize = {0};
		tk_tokenize(temp.arena, &tokenize, source);

		StringList actual_output_list = {0};
		string_list_pushf(temp.arena, &actual_output_list, "%td bytes\n", source.length);
		tk_tokenize_result_stringify(temp.arena, tokenize, &actual_output_list);
		String actual_output = string_list_join(temp.arena, actual_output_list);

		if (string_equal(expected_output, actual_output)) {
			printf("%.*s succeeded.\n", str_fmt(entry->path));
		} else {
			printf("%.*s failed.\n", str_fmt(entry->path));
			printf("expected:\n%.*s", str_fmt(expected_output));
			printf("actual:\n%.*s", str_fmt(actual_output));
		}
	}

	temp_end(temp);
}
