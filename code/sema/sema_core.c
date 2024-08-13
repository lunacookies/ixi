typedef struct SM_Context SM_Context;
struct SM_Context {
	SM_Procedure *first_procedure;
	SM_Procedure *last_procedure;
	D_DiagnosticList diagnostics;
};

function SM_Expression *
sm_analyze_expression(Arena *arena, SM_Context *cx, P_Expression *expression)
{
	SM_Expression *result = push_struct(arena, SM_Expression);

	switch (expression->kind) {
	case P_ExpressionKind_Number: {
		f64 number = expression->number;
		result->kind = SM_ExpressionKind_Number;
		result->number = number;
		break;
	}

	case P_ExpressionKind_Unary: {
		P_UnaryExpression unary = expression->unary;
		result->kind = SM_ExpressionKind_Unary;
		result->unary.op = unary.op;
		result->unary.operand = sm_analyze_expression(arena, cx, unary.operand);
		break;
	}

	case P_ExpressionKind_Binary: {
		P_BinaryExpression binary = expression->binary;
		result->kind = SM_ExpressionKind_Binary;
		result->binary.lhs = sm_analyze_expression(arena, cx, binary.lhs);
		result->binary.rhs = sm_analyze_expression(arena, cx, binary.rhs);
		result->binary.op = binary.op;
		break;
	}

	case P_ExpressionKind_Invalid: break;
	}

	return result;
}

function SM_Statement *
sm_analyze_statement(Arena *arena, SM_Context *cx, P_Statement *statement)
{
	SM_Statement *result = push_struct(arena, SM_Statement);

	switch (statement->kind) {
	case P_StatementKind_Expression: {
		P_Expression *expression = statement->expression;
		result->kind = SM_StatementKind_Expression;
		result->expression = sm_analyze_expression(arena, cx, expression);
		break;
	}

	case P_StatementKind_Invalid: break;
	}

	return result;
}

function SM_Statement *
sm_analyze_statements(Arena *arena, SM_Context *cx, P_Statement *statements)
{
	SM_Statement *first = 0;
	SM_Statement *last = 0;

	for (P_Statement *statement = statements; statement != 0; statement = statement->next) {
		SM_Statement *analyzed = sm_analyze_statement(arena, cx, statement);
		if (first == 0) {
			assert(last == 0);
			first = analyzed;
			last = analyzed;
		} else {
			assert(last != 0);
			last->next = analyzed;
			last = analyzed;
		}
	}

	return first;
}

function void
sm_analyze_procedure(Arena *arena, SM_Context *cx, P_Procedure *procedure)
{
	SM_Procedure *result = push_struct(arena, SM_Procedure);
	result->name = string_clone(arena, procedure->name);

	result->body = sm_analyze_statements(arena, cx, procedure->body);

	if (cx->first_procedure == 0) {
		assert(cx->last_procedure == 0);
		cx->first_procedure = result;
		cx->last_procedure = result;
	} else {
		assert(cx->last_procedure != 0);
		cx->last_procedure->next = result;
		cx->last_procedure = result;
	}
}

function void
sm_analyze(Arena *arena, SM_AnalysisResult *result, P_ParseResult parse)
{
	memory_zero_struct(result);

	SM_Context cx = {0};

	for (P_Entity *entity = parse.root.first_entity; entity != 0; entity = entity->next) {
		if (entity->kind != P_EntityKind_Procedure) {
			continue;
		}

		P_Procedure *procedure = &entity->procedure;
		sm_analyze_procedure(arena, &cx, procedure);
	}

	result->first_procedure = cx.first_procedure;
	result->diagnostics = cx.diagnostics;
}
