enum {
	tk_token_chunk_capacity = 1024,
};

typedef struct TK_Chunk TK_Chunk;
struct TK_Chunk {
	TK_Chunk *next;
	isize token_count;
	TK_TokenKind kinds[tk_token_chunk_capacity];
	D_Span spans[tk_token_chunk_capacity];
};

typedef struct TK_Tokenizer TK_Tokenizer;
struct TK_Tokenizer {
	String source;
	isize cursor;
	u8 byte;

	isize token_count;
	TK_Chunk *first_chunk;
	TK_Chunk *last_chunk;

	D_DiagnosticList diagnostics;
};

function void
tk_advance(TK_Tokenizer *tokenizer)
{
	tokenizer->cursor++;
	assert(tokenizer->cursor <= tokenizer->source.length);
	if (tokenizer->cursor == tokenizer->source.length) {
		tokenizer->byte = 0;
	} else {
		tokenizer->byte = tokenizer->source.data[tokenizer->cursor];
	}
}

function void
tk_emit(Arena *temp_arena, TK_Tokenizer *tokenizer, TK_TokenKind kind, isize start, isize end)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(start <= end);

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
			tokenizer->last_chunk->next = chunk;
			tokenizer->last_chunk = chunk;
		} else {
			chunk = tokenizer->last_chunk;
		}
	}

	D_Span span = {(s32)start, (s32)end};
	chunk->kinds[chunk->token_count] = kind;
	chunk->spans[chunk->token_count] = span;
	chunk->token_count++;
	tokenizer->token_count++;
}

function void
tk_error(Arena *arena, TK_Tokenizer *tokenizer, isize start, isize end, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	assert(start >= 0);
	assert(end >= 0);
	assert(start <= end);

	D_Diagnostic *diagnostic = d_diagnostic_list_push(arena, &tokenizer->diagnostics);
	diagnostic->span.start = (s32)start;
	diagnostic->span.end = (s32)end;
	diagnostic->severity = D_Severity_Error;
	diagnostic->message = push_stringfv(arena, fmt, ap);

	va_end(ap);
}

function b32
tk_at_whitespace(TK_Tokenizer *t)
{
	return t->byte == ' ' || t->byte == '\t' || t->byte == '\n';
}

function b32
tk_at_identifier(TK_Tokenizer *t)
{
	return (t->byte >= 'a' && t->byte <= 'z') || (t->byte >= 'A' && t->byte <= 'Z') ||
	       t->byte == '_';
}

function b32
tk_at_number(TK_Tokenizer *t)
{
	return t->byte >= '0' && t->byte <= '9';
}

function b32
tk_at_symbol2(TK_Tokenizer *t)
{
	u8 byte = t->byte;
	u8 byte2 = t->source.data[t->cursor + 1];
	return (byte == '<' && byte2 == '=') || (byte == '>' && byte2 == '=') ||
	       (byte == '|' && byte2 == '|') || (byte == '&' && byte2 == '&') ||
	       (byte == '=' && byte2 == '=') || (byte == '!' && byte2 == '=') ||
	       (byte == '<' && byte2 == '<') || (byte == '>' && byte2 == '>') ||
	       (byte == '-' && byte2 == '>');
}

function b32
tk_at_symbol(TK_Tokenizer *t)
{
	switch (t->byte) {
	case '!':
	case '#':
	case '%':
	case '&':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case '-':
	case '.':
	case '/':
	case ':':
	case ';':
	case '<':
	case '=':
	case '>':
	case '[':
	case ']':
	case '^':
	case '{':
	case '|':
	case '}':
	case '~': return 1;
	default: return 0;
	}
}

function TK_TokenKind
tk_token_kind_for_symbol(u8 symbol)
{
	switch (symbol) {
	case '!': return TK_TokenKind_Bang;
	case '#': return TK_TokenKind_Hash;
	case '%': return TK_TokenKind_Percent;
	case '&': return TK_TokenKind_Ampersand;
	case '(': return TK_TokenKind_LParen;
	case ')': return TK_TokenKind_RParen;
	case '*': return TK_TokenKind_Asterisk;
	case '+': return TK_TokenKind_Plus;
	case ',': return TK_TokenKind_Comma;
	case '-': return TK_TokenKind_Hyphen;
	case '.': return TK_TokenKind_Period;
	case '/': return TK_TokenKind_Slash;
	case ':': return TK_TokenKind_Colon;
	case ';': return TK_TokenKind_Semi;
	case '<': return TK_TokenKind_LAngle;
	case '=': return TK_TokenKind_Equal;
	case '>': return TK_TokenKind_RAngle;
	case '[': return TK_TokenKind_LSquare;
	case ']': return TK_TokenKind_RSquare;
	case '^': return TK_TokenKind_Caret;
	case '{': return TK_TokenKind_LBrace;
	case '|': return TK_TokenKind_Pipe;
	case '}': return TK_TokenKind_RBrace;
	case '~': return TK_TokenKind_Tilde;
	default: assert(0); return 0;
	}
}

function b32
tk_at_valid(TK_Tokenizer *t)
{
	return tk_at_whitespace(t) || tk_at_identifier(t) || tk_at_number(t) || tk_at_symbol2(t) ||
	       tk_at_symbol(t);
}

global read_only struct {
	String name;
	TK_TokenKind kind;
} tk_keyword_table[] = {
        {str_lit("proc"), TK_TokenKind_ProcKw},
        {str_lit("struct"), TK_TokenKind_StructKw},
        {str_lit("const"), TK_TokenKind_ConstKw},
        {str_lit("var"), TK_TokenKind_VarKw},
        {str_lit("if"), TK_TokenKind_IfKw},
        {str_lit("else"), TK_TokenKind_ElseKw},
        {str_lit("for"), TK_TokenKind_ForKw},
        {str_lit("break"), TK_TokenKind_BreakKw},
        {str_lit("continue"), TK_TokenKind_ContinueKw},
        {str_lit("switch"), TK_TokenKind_SwitchKw},
        {str_lit("case"), TK_TokenKind_CaseKw},
        {str_lit("return"), TK_TokenKind_ReturnKw},
};

enum {
	tk_virtual_semi_kinds = (1ll << TK_TokenKind_Identifier) | (1ll << TK_TokenKind_Number),
};

function void
tk_eat_token(Arena *arena, Arena *temp_arena, TK_Tokenizer *t)
{
	if (tk_at_whitespace(t)) {
		assert(tk_at_valid(t));
		b32 newline = t->byte == '\n';
		tk_advance(t);

		if (newline && t->token_count > 0) {
			TK_TokenKind last_token_kind =
			        t->last_chunk->kinds[t->last_chunk->token_count - 1];
			D_Span last_token_span =
			        t->last_chunk->spans[t->last_chunk->token_count - 1];

			if (((1ll << last_token_kind) & tk_virtual_semi_kinds) != 0) {
				tk_emit(temp_arena, t, TK_TokenKind_Semi, last_token_span.end,
				        last_token_span.end);
			}
		}

		return;
	}

	if (tk_at_identifier(t)) {
		assert(tk_at_valid(t));
		isize start = t->cursor;

		for (; t->cursor < t->source.length; tk_advance(t)) {
			if ((t->byte >= 'a' && t->byte <= 'z') ||
			        (t->byte >= 'A' && t->byte <= 'Z') ||
			        (t->byte >= '0' && t->byte <= '9') || t->byte == '_') {
				continue;
			}

			break;
		}

		isize end = t->cursor;

		String identifier = string_slice(t->source, start, end);
		TK_TokenKind kind = TK_TokenKind_Identifier;
		for (isize i = 0; i < array_count(tk_keyword_table); i++) {
			if (string_equal(identifier, tk_keyword_table[i].name)) {
				kind = tk_keyword_table[i].kind;
				break;
			}
		}

		tk_emit(temp_arena, t, kind, start, end);
		return;
	}

	if (tk_at_number(t)) {
		assert(tk_at_valid(t));
		isize start = t->cursor;
		b32 seen_decimal_point = 0;

		for (; t->cursor < t->source.length; tk_advance(t)) {
			if (t->byte >= '0' && t->byte <= '9') {
				continue;
			}

			if (t->byte == '.' && !seen_decimal_point) {
				seen_decimal_point = 1;
				continue;
			}

			break;
		}

		isize end = t->cursor;

		tk_emit(temp_arena, t, TK_TokenKind_Number, start, end);
		return;
	}

	if (tk_at_symbol2(t)) {
		assert(tk_at_valid(t));
		isize start = t->cursor;
		u8 symbol = t->byte;
		tk_advance(t);
		u8 symbol2 = t->byte;
		tk_advance(t);
		isize end = t->cursor;

		TK_TokenKind kind = 0;
		if (symbol == '<' && symbol2 == '=') {
			kind = TK_TokenKind_LAngleEqual;
		} else if (symbol == '>' && symbol2 == '=') {
			kind = TK_TokenKind_RAngleEqual;
		} else if (symbol == '|' && symbol2 == '|') {
			kind = TK_TokenKind_Pipe2;
		} else if (symbol == '&' && symbol2 == '&') {
			kind = TK_TokenKind_Ampersand2;
		} else if (symbol == '=' && symbol2 == '=') {
			kind = TK_TokenKind_Equal2;
		} else if (symbol == '!' && symbol2 == '=') {
			kind = TK_TokenKind_BangEqual;
		} else if (symbol == '<' && symbol2 == '<') {
			kind = TK_TokenKind_LAngle2;
		} else if (symbol == '>' && symbol2 == '>') {
			kind = TK_TokenKind_RAngle2;
		} else if (symbol == '-' && symbol2 == '>') {
			kind = TK_TokenKind_Arrow;
		}

		tk_emit(temp_arena, t, kind, start, end);
		return;
	}

	if (tk_at_symbol(t)) {
		assert(tk_at_valid(t));
		isize start = t->cursor;
		u8 symbol = t->byte;
		tk_advance(t);
		isize end = t->cursor;

		tk_emit(temp_arena, t, tk_token_kind_for_symbol(symbol), start, end);
		return;
	}

	isize start = t->cursor;

	for (; t->cursor < t->source.length && !tk_at_valid(t);) {
		tk_advance(t);
	}

	isize end = t->cursor;

	tk_emit(temp_arena, t, TK_TokenKind_Error, start, end);
	tk_error(arena, t, start, end, "unrecognized sequence");
}

function void
tk_tokenize(Arena *arena, TK_TokenizeResult *result, String source)
{
	memory_zero_struct(result);

	TK_Tokenizer tokenizer = {0};
	tokenizer.source = source;
	if (source.length > 0) {
		tokenizer.byte = source.data[0];
	}

	Temp temp = temp_begin(&arena, 1);

	for (; tokenizer.cursor < tokenizer.source.length;) {
		tk_eat_token(arena, temp.arena, &tokenizer);
	}

	TK_TokenKind *kinds = push_array(arena, TK_TokenKind, tokenizer.token_count);
	D_Span *spans = push_array(arena, D_Span, tokenizer.token_count);

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

	result->diagnostics = tokenizer.diagnostics;
}
