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

enum {
	p_entity_first = 1ll << TK_TokenKind_ProcKw,
};

function isize
p_tightness(P_BinaryOperator op)
{
	switch (op) {
	case P_BinaryOperator_Multiply:
	case P_BinaryOperator_Divide:
	case P_BinaryOperator_Modulo:
	case P_BinaryOperator_BitAnd:
	case P_BinaryOperator_ShiftLeft:
	case P_BinaryOperator_ShiftRight: return 5;

	case P_BinaryOperator_Add:
	case P_BinaryOperator_Subtract:
	case P_BinaryOperator_BitOr:
	case P_BinaryOperator_BitXor: return 4;

	case P_BinaryOperator_Equal:
	case P_BinaryOperator_NotEqual:
	case P_BinaryOperator_LessThan:
	case P_BinaryOperator_GreaterThan:
	case P_BinaryOperator_LessThanEqual:
	case P_BinaryOperator_GreaterThanEqual: return 3;

	case P_BinaryOperator_And: return 2;

	case P_BinaryOperator_Or: return 1;

	case P_BinaryOperator_Invalid: return 0;
	}
}

function b32
p_right_binds_tighter(P_BinaryOperator left, P_BinaryOperator right)
{
	isize left_tightness = p_tightness(left);
	isize right_tightness = p_tightness(right);
	return right_tightness > left_tightness;
}

function b32
p_at_eof(P_Parser *p)
{
	assert(p->cursor <= p->token_count);
	return p->cursor == p->token_count;
}

function TK_TokenKind
p_current(P_Parser *p)
{
	if (p_at_eof(p)) {
		return TK_TokenKind_EOF;
	}

	assert(p->cursor <= p->token_count);
	return p->kinds[p->cursor];
}

function b32
p_at(P_Parser *p, TK_TokenKind kind)
{
	return p_current(p) == kind;
}

function b32
p_at_set(P_Parser *p, u64 set)
{
	return ((1ll << p_current(p)) & set) != 0;
}

function D_Span
p_current_span(P_Parser *p)
{
	D_Span result = {0};

	if (p_at_eof(p)) {
		if (p->token_count > 0) {
			D_Span last_span = p->spans[p->token_count - 1];
			result.start = last_span.end;
			result.end = last_span.end;
		}
	} else {
		assert(p->cursor < p->token_count);
		result = p->spans[p->cursor];
	}

	return result;
}

function D_Span
p_previous_span(P_Parser *p)
{
	D_Span result = {0};

	if (p->cursor > 0) {
		p->cursor--;
		result = p_current_span(p);
		p->cursor++;
	}

	return result;
}

function void
p_error_ext(Arena *arena, P_Parser *p, b32 collapse_span, char *fmt, va_list ap)
{
	D_Diagnostic *diagnostic = d_diagnostic_list_push(arena, &p->diagnostics);
	diagnostic->severity = D_Severity_Error;
	diagnostic->message = push_stringfv(arena, fmt, ap);

	diagnostic->span = p_current_span(p);
	if (collapse_span) {
		diagnostic->span.end = diagnostic->span.start;
	}
}

function void __attribute__((format(printf, 3, 4)))
p_error(Arena *arena, P_Parser *p, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	p_error_ext(arena, p, 0, fmt, ap);
	va_end(ap);
}

function void __attribute__((format(printf, 3, 4)))
p_error_point(Arena *arena, P_Parser *p, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	p_error_ext(arena, p, 1, fmt, ap);
	va_end(ap);
}

function String
p_bump(P_Parser *p, TK_TokenKind kind)
{
	assert(p_at(p, kind));
	D_Span span = p_current_span(p);
	p->cursor++;
	return string_slice(p->source, span.start, span.end);
}

function String
p_expect_name(Arena *arena, P_Parser *p, TK_TokenKind kind, u64 recovery_set, String name)
{
	if (p_at(p, kind)) {
		return p_bump(p, kind);
	}

	if (name.data == 0) {
		name = tk_token_kind_human_names[kind];
	}

	recovery_set |= 1ll << TK_TokenKind_LBrace;
	recovery_set |= 1ll << TK_TokenKind_Semi;
	recovery_set |= p_entity_first;
	b32 should_recover = p_at_set(p, recovery_set);
	if (should_recover || kind == TK_TokenKind_Semi || p_at_eof(p)) {
		p_error_point(arena, p, "missing %.*s", str_fmt(name));
	} else {
		p_error(arena, p, "expected %.*s but found %.*s", str_fmt(name),
		        str_fmt(tk_token_kind_human_names[p_current(p)]));
		p->cursor++;
	}

	String result = {0};
	return result;
}

function String
p_expect(Arena *arena, P_Parser *p, TK_TokenKind kind, u64 recovery_set)
{
	return p_expect_name(arena, p, kind, recovery_set, (String){0});
}

function P_Entity *
p_push_entity(Arena *arena, P_Parser *p)
{
	P_Entity *entity = push_struct(arena, P_Entity);

	if (p->root.first_entity == 0) {
		assert(p->root.last_entity == 0);
		p->root.first_entity = entity;
		p->root.last_entity = entity;
	} else {
		assert(p->root.last_entity != 0);
		p->root.last_entity->next = entity;
		p->root.last_entity = entity;
	}

	return entity;
}

function P_Expression *p_parse_expression(Arena *arena, P_Parser *p);

function P_Expression *
p_parse_atom(Arena *arena, P_Parser *p)
{
	D_Span start_span = p_current_span(p);
	P_Expression *expression = push_struct(arena, P_Expression);

	switch (p_current(p)) {
	case TK_TokenKind_Number: {
		expression->kind = P_ExpressionKind_Number;
		String text = p_bump(p, TK_TokenKind_Number);
		expression->data.number = f64_from_string(text);
		break;
	}

	default: p_error(arena, p, "expected expression"); break;
	}

	D_Span end_span = p_previous_span(p);
	expression->span.start = start_span.start;
	expression->span.end = end_span.end;
	return expression;
}

function P_Expression *
p_parse_lhs(Arena *arena, P_Parser *p)
{
	P_Expression *result = 0;

	switch (p_current(p)) {
	case TK_TokenKind_Plus:
	case TK_TokenKind_Hyphen: {
		TK_TokenKind kind = p_current(p);
		P_UnaryOperator op = 0;
		switch (kind) {
		case TK_TokenKind_Plus: op = P_UnaryOperator_Identity; break;
		case TK_TokenKind_Hyphen: op = P_UnaryOperator_Negate; break;
		default: break;
		}

		D_Span start_span = p_current_span(p);
		p_bump(p, kind);
		P_Expression *operand = p_parse_lhs(arena, p);
		D_Span end_span = p_previous_span(p);

		result = push_struct(arena, P_Expression);
		result->kind = P_ExpressionKind_Unary;
		result->data.unary.op = op;
		result->data.unary.operand = operand;
		result->span.start = start_span.start;
		result->span.end = end_span.end;
		break;
	}

	case TK_TokenKind_LParen: {
		p_bump(p, TK_TokenKind_LParen);
		result = p_parse_expression(arena, p);
		p_expect(arena, p, TK_TokenKind_RParen, 0);
		break;
	}

	default: result = p_parse_atom(arena, p); break;
	}

	return result;
}

function P_Expression *
p_parse_expression_rec(Arena *arena, P_Parser *p, P_BinaryOperator left)
{
	P_Expression *lhs = p_parse_lhs(arena, p);

	for (; !p_at_set(p, p_entity_first) && !p_at_eof(p);) {
		P_BinaryOperator right = 0;
		switch (p_current(p)) {
		case TK_TokenKind_Plus: right = P_BinaryOperator_Add; break;
		case TK_TokenKind_Hyphen: right = P_BinaryOperator_Subtract; break;
		case TK_TokenKind_Asterisk: right = P_BinaryOperator_Multiply; break;
		case TK_TokenKind_Slash: right = P_BinaryOperator_Divide; break;
		case TK_TokenKind_Percent: right = P_BinaryOperator_Modulo; break;
		case TK_TokenKind_Ampersand: right = P_BinaryOperator_BitAnd; break;
		case TK_TokenKind_Pipe: right = P_BinaryOperator_BitOr; break;
		case TK_TokenKind_Caret: right = P_BinaryOperator_BitXor; break;
		case TK_TokenKind_LAngle2: right = P_BinaryOperator_ShiftLeft; break;
		case TK_TokenKind_RAngle2: right = P_BinaryOperator_ShiftRight; break;
		case TK_TokenKind_Equal2: right = P_BinaryOperator_Equal; break;
		case TK_TokenKind_BangEqual: right = P_BinaryOperator_NotEqual; break;
		case TK_TokenKind_LAngle: right = P_BinaryOperator_LessThan; break;
		case TK_TokenKind_RAngle: right = P_BinaryOperator_GreaterThan; break;
		case TK_TokenKind_LAngleEqual: right = P_BinaryOperator_LessThanEqual; break;
		case TK_TokenKind_RAngleEqual: right = P_BinaryOperator_GreaterThanEqual; break;
		case TK_TokenKind_Ampersand2: right = P_BinaryOperator_And; break;
		case TK_TokenKind_Pipe2: right = P_BinaryOperator_Or; break;
		default: return lhs;
		}

		if (!p_right_binds_tighter(left, right)) {
			break;
		}

		p_bump(p, p_current(p));

		P_Expression *rhs = p_parse_expression_rec(arena, p, right);

		P_Expression *new_lhs = push_struct(arena, P_Expression);
		new_lhs->kind = P_ExpressionKind_Binary;
		new_lhs->span.start = lhs->span.start;
		new_lhs->span.end = p_previous_span(p).end;

		P_BinaryExpression *binary = &new_lhs->data.binary;
		binary->lhs = lhs;
		binary->rhs = rhs;
		binary->op = right;

		lhs = new_lhs;
	}

	return lhs;
}

function P_Expression *
p_parse_expression(Arena *arena, P_Parser *p)
{
	return p_parse_expression_rec(arena, p, P_BinaryOperator_Invalid);
}

function P_Statement *
p_parse_statement(Arena *arena, P_Parser *p)
{
	D_Span start_span = p_current_span(p);
	P_Statement *statement = push_struct(arena, P_Statement);

	switch (p_current(p)) {
	case TK_TokenKind_Number:
	case TK_TokenKind_Plus:
	case TK_TokenKind_Hyphen:
	case TK_TokenKind_LParen:
		statement->kind = P_StatementKind_Expression;
		statement->data.expression = p_parse_expression(arena, p);
		p_expect(arena, p, TK_TokenKind_Semi, 0);
		break;

	default: p_error(arena, p, "expected statement"); break;
	}

	D_Span end_span = p_previous_span(p);
	statement->span.start = start_span.start;
	statement->span.end = end_span.end;
	return statement;
}

function void
p_parse_root(Arena *arena, P_Parser *p)
{
	for (; !p_at_eof(p);) {
		p_expect(arena, p, TK_TokenKind_ProcKw, 0);

		P_Entity *entity = p_push_entity(arena, p);
		entity->kind = P_EntityKind_Procedure;
		P_Procedure *procedure = &entity->data.procedure;

		procedure->name = p_expect_name(arena, p, TK_TokenKind_Identifier,
		        1ll << TK_TokenKind_LParen, str_lit("procedure name"));

		p_expect(arena, p, TK_TokenKind_LParen, 0);
		p_expect(arena, p, TK_TokenKind_RParen, 0);

		P_Statement *first_statement = 0;
		P_Statement *last_statement = 0;

		p_expect(arena, p, TK_TokenKind_LBrace, 0);

		for (; !p_at(p, TK_TokenKind_RBrace) && !p_at_set(p, p_entity_first) &&
		        !p_at_eof(p);) {
			P_Statement *statement = p_parse_statement(arena, p);
			if (first_statement == 0) {
				assert(last_statement == 0);
				first_statement = statement;
				last_statement = statement;
			} else {
				assert(last_statement != 0);
				last_statement->next = statement;
				last_statement = statement;
			}
		}

		procedure->body = first_statement;

		p_expect(arena, p, TK_TokenKind_RBrace, 0);
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
