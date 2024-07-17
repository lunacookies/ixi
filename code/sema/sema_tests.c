function void
sm_newline_print(Arena *arena, StringList *list, isize *tabs)
{
	string_list_push(arena, list, str_lit("\n"));
	for (isize i = 0; i < *tabs; i++) {
		string_list_push(arena, list, str_lit("\t"));
	}
}

function void
sm_expression_print(Arena *arena, StringList *list, SM_Expression *expression)
{
	switch (expression->kind) {
	case SM_ExpressionKind_Number: {
		f64 number = expression->data.number;
		string_list_pushf(arena, list, "%g", number);
		break;
	}

	case SM_ExpressionKind_Unary: {
		SM_UnaryExpression *unary = &expression->data.unary;
		string_list_push(arena, list, str_lit("("));

		String op = {0};
		switch (unary->op) {
		case P_UnaryOperator_Identity: op = str_lit("+"); break;
		case P_UnaryOperator_Negate: op = str_lit("-"); break;
		case P_UnaryOperator_BitComplement: op = str_lit("^"); break;
		case P_UnaryOperator_Invalid: op = str_lit("<invalid operator>"); break;
		}

		string_list_push(arena, list, op);
		sm_expression_print(arena, list, unary->operand);

		string_list_push(arena, list, str_lit(")"));
		break;
	}

	case SM_ExpressionKind_Binary: {
		SM_BinaryExpression *binary = &expression->data.binary;
		string_list_push(arena, list, str_lit("("));

		sm_expression_print(arena, list, binary->lhs);

		string_list_push(arena, list, str_lit(" "));

		String op = {0};
		switch (binary->op) {
		case P_BinaryOperator_Add: op = str_lit("+"); break;
		case P_BinaryOperator_Subtract: op = str_lit("-"); break;
		case P_BinaryOperator_Multiply: op = str_lit("*"); break;
		case P_BinaryOperator_Divide: op = str_lit("/"); break;
		case P_BinaryOperator_Modulo: op = str_lit("%"); break;
		case P_BinaryOperator_BitAnd: op = str_lit("&"); break;
		case P_BinaryOperator_BitOr: op = str_lit("|"); break;
		case P_BinaryOperator_BitXor: op = str_lit("^"); break;
		case P_BinaryOperator_ShiftLeft: op = str_lit("<<"); break;
		case P_BinaryOperator_ShiftRight: op = str_lit(">>"); break;
		case P_BinaryOperator_Equal: op = str_lit("=="); break;
		case P_BinaryOperator_NotEqual: op = str_lit("!="); break;
		case P_BinaryOperator_LessThan: op = str_lit("<"); break;
		case P_BinaryOperator_GreaterThan: op = str_lit(">"); break;
		case P_BinaryOperator_LessThanEqual: op = str_lit("<="); break;
		case P_BinaryOperator_GreaterThanEqual: op = str_lit(">="); break;
		case P_BinaryOperator_And: op = str_lit("&&"); break;
		case P_BinaryOperator_Or: op = str_lit("||"); break;
		case P_BinaryOperator_Invalid: op = str_lit("<invalid operator>"); break;
		}

		string_list_push(arena, list, op);
		string_list_push(arena, list, str_lit(" "));

		sm_expression_print(arena, list, binary->rhs);

		string_list_push(arena, list, str_lit(")"));
		break;
	}

	case SM_ExpressionKind_Invalid:
		string_list_push(arena, list, str_lit("<invalid expression>"));
		break;
	}
}

function void
sm_statement_print(Arena *arena, StringList *list, SM_Statement *statement)
{
	switch (statement->kind) {
	case SM_StatementKind_Expression: {
		SM_Expression *expression = statement->data.expression;
		sm_expression_print(arena, list, expression);
		string_list_push(arena, list, str_lit(";"));
		break;
	}

	case SM_StatementKind_Invalid:
		string_list_push(arena, list, str_lit("<invalid statement>;"));
		break;
	}
}

function void
sm_procedure_print(Arena *arena, StringList *list, SM_Procedure *procedure, isize *tabs)
{
	string_list_push(arena, list, str_lit("proc "));

	if (procedure->name.data == 0) {
		string_list_push(arena, list, str_lit("<missing>"));
	} else {
		string_list_push(arena, list, procedure->name);
	}

	string_list_push(arena, list, str_lit("() {"));

	if (procedure->body != 0) {
		(*tabs)++;

		for (SM_Statement *stmt = procedure->body; stmt != 0; stmt = stmt->next) {
			sm_newline_print(arena, list, tabs);
			sm_statement_print(arena, list, stmt);
		}

		(*tabs)--;
		sm_newline_print(arena, list, tabs);
	}

	string_list_push(arena, list, str_lit("}"));

	sm_newline_print(arena, list, tabs);
}

function String
sm_analysis_result_stringify(Arena *arena, SM_AnalysisResult analysis)
{
	Temp temp = temp_begin(&arena, 1);

	StringList list = {0};
	isize tabs = 0;

	for (SM_Procedure *procedure = analysis.first_procedure; procedure != 0;
	        procedure = procedure->next) {
		sm_procedure_print(temp.arena, &list, procedure, &tabs);
	}

	string_list_pushf(temp.arena, &list, "%td errors:\n", analysis.diagnostics.count);
	for (D_DiagnosticNode *node = analysis.diagnostics.first; node != 0; node = node->next) {
		string_list_push(temp.arena, &list, str_lit("    "));
		d_diagnostic_print(temp.arena, node->diagnostic, &list);
		string_list_push(temp.arena, &list, str_lit("\n"));
	}

	String result = string_list_join(arena, list);
	temp_end(temp);
	return result;
}

function void
sm_tests(void)
{
	Temp temp = temp_begin(0, 0);

	String sep = str_lit("===\n");
	b32 update_tests = os_env_get(temp.arena, str_lit("UPDATE_TESTS")).data != 0;

	OS_Entry *entries = os_directory_entries(temp.arena, str_lit("code/sema/test_data"));

	for (OS_Entry *entry = entries; entry != 0; entry = entry->next) {
		if (entry->is_directory) {
			continue;
		}

		String contents = os_read_file(temp.arena, entry->path);

		String source = {0};
		String expected_output = {0};
		b32 found = string_cut(contents, &source, &expected_output, sep);
		assert(found);

		P_ParseResult parse = {0};
		p_parse(temp.arena, &parse, source);

		SM_AnalysisResult analysis = {0};
		sm_analyze(temp.arena, &analysis, parse);

		String actual_output = sm_analysis_result_stringify(temp.arena, analysis);

		if (string_equal(expected_output, actual_output)) {
			printf("%.*s succeeded.\n", str_fmt(entry->path));
		} else {
			printf("%.*s failed.\n", str_fmt(entry->path));
			printf("expected:\n%.*s", str_fmt(expected_output));
			printf("actual:\n%.*s", str_fmt(actual_output));

			if (update_tests) {
				String new_file_contents = push_stringf(temp.arena, "%.*s%.*s%.*s",
				        str_fmt(source), str_fmt(sep), str_fmt(actual_output));
				os_write_file(entry->path, new_file_contents);
			}
		}
	}

	temp_end(temp);
}
