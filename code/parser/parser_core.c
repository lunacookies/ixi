typedef struct P_Parser P_Parser;
struct P_Parser {
	String source;

	isize token_count;
	isize cursor;
	TK_TokenKind *kinds;
	D_Span *spans;

	P_Root root;
	D_DiagnosticList diagnostics;
};

function b32
p_at_end(P_Parser *p)
{
	assert(p->cursor <= p->token_count);
	return p->cursor == p->token_count;
}

function TK_TokenKind
p_current(P_Parser *p)
{
	if (p_at_end(p)) {
		return TK_TokenKind_EOF;
	}

	assert(p->cursor <= p->token_count);
	return p->kinds[p->cursor];
}

function String
p_expect_name(Arena *arena, P_Parser *p, TK_TokenKind kind, u64 recovery_set, String name)
{
	TK_TokenKind actual_kind = p_current(p);

	D_Span span = {0};
	if (p_at_end(p)) {
		if (p->token_count > 0) {
			D_Span last_span = p->spans[p->token_count - 1];
			span.start = last_span.end;
			span.end = last_span.end;
		}
	} else {
		assert(p->cursor < p->token_count);
		span = p->spans[p->cursor];
	}

	if (actual_kind == kind) {
		p->cursor++;
		return string_slice(p->source, span.start, span.end);
	}

	D_Diagnostic *diagnostic = d_diagnostic_list_push(arena, &p->diagnostics);

	if (name.data == 0) {
		name = tk_token_kind_human_names[kind];
	}

	recovery_set |= 1ll << TK_TokenKind_LBrace;
	b32 should_recover = ((1ll << actual_kind) & recovery_set) != 0;
	if (should_recover) {
		span.end = span.start;
		diagnostic->span = span;
		diagnostic->severity = D_Severity_Error;
		diagnostic->message = push_stringf(arena, "missing %.*s", str_fmt(name));
	} else {
		diagnostic->span = span;
		diagnostic->severity = D_Severity_Error;
		diagnostic->message = push_stringf(arena, "expected %.*s but found %.*s",
		        str_fmt(name), str_fmt(tk_token_kind_human_names[actual_kind]));

		if (!p_at_end(p)) {
			p->cursor++;
		}
	}

	String result = {0};
	return result;
}

function String
p_expect(Arena *arena, P_Parser *p, TK_TokenKind kind, u64 recovery_set)
{
	return p_expect_name(arena, p, kind, recovery_set, (String){0});
}

function void
p_parse_root(Arena *arena, P_Parser *p)
{
	for (; !p_at_end(p);) {
		p_expect(arena, p, TK_TokenKind_ProcKw, 0);
		String name = p_expect_name(arena, p, TK_TokenKind_Identifier,
		        1ll << TK_TokenKind_LParen, str_lit("procedure name"));
		p_expect(arena, p, TK_TokenKind_LParen, 0);
		p_expect(arena, p, TK_TokenKind_RParen, 0);
		p_expect(arena, p, TK_TokenKind_LBrace, 0);
		p_expect(arena, p, TK_TokenKind_RBrace, 0);

		P_Entity *entity = push_struct(arena, P_Entity);
		entity->kind = P_EntityKind_Procedure;
		P_Procedure *procedure = &entity->data.procedure;
		procedure->name = name;

		if (p->root.first_entity == 0) {
			assert(p->root.last_entity == 0);
			p->root.first_entity = entity;
			p->root.last_entity = entity;
		} else {
			assert(p->root.last_entity != 0);
			p->root.last_entity->next = entity;
			p->root.last_entity = entity;
		}
	}
}

function void
p_parse(Arena *arena, P_ParseResult *result, String source)
{
	TK_TokenizeResult tokenize = {0};
	tk_tokenize(arena, &tokenize, source);

	P_Parser parser = {0};
	parser.source = source;
	parser.token_count = tokenize.token_count;
	parser.kinds = tokenize.kinds;
	parser.spans = tokenize.spans;

	p_parse_root(arena, &parser);

	result->root = parser.root;
	result->diagnostics = parser.diagnostics;
}
