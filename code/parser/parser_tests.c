function void
p_newline_print(Arena *arena, StringList *list, isize *tabs)
{
	string_list_push(arena, list, str_lit("\n"));
	for (isize i = 0; i < *tabs; i++) {
		string_list_push(arena, list, str_lit("\t"));
	}
}

function void
p_expression_print(Arena *arena, StringList *list, P_Expression *expression)
{
	switch (expression->kind) {
	case P_ExpressionKind_Number: {
		f64 number = expression->data.number;
		string_list_pushf(arena, list, "%g", number);
		break;
	}

	case P_ExpressionKind_Unary: {
		P_UnaryExpression *unary = &expression->data.unary;
		string_list_push(arena, list, str_lit("("));

		String op = {0};
		switch (unary->op) {
		case P_UnaryOperator_Negate: op = str_lit("-"); break;
		case P_UnaryOperator_Invalid: op = str_lit("<invalid operator>"); break;
		}

		string_list_push(arena, list, op);
		p_expression_print(arena, list, unary->operand);

		string_list_push(arena, list, str_lit(")"));
		break;
	}

	case P_ExpressionKind_Binary: {
		P_BinaryExpression *binary = &expression->data.binary;
		string_list_push(arena, list, str_lit("("));

		p_expression_print(arena, list, binary->lhs);

		string_list_push(arena, list, str_lit(" "));

		String op = {0};
		switch (binary->op) {
		case P_BinaryOperator_Add: op = str_lit("+"); break;
		case P_BinaryOperator_Multiply: op = str_lit("*"); break;
		case P_BinaryOperator_Invalid: op = str_lit("<invalid operator>"); break;
		}

		string_list_push(arena, list, op);
		string_list_push(arena, list, str_lit(" "));

		p_expression_print(arena, list, binary->rhs);

		string_list_push(arena, list, str_lit(")"));
		break;
	}

	case P_ExpressionKind_Invalid:
		string_list_push(arena, list, str_lit("<invalid expression>"));
		break;
	}
}

function void
p_statement_print(Arena *arena, StringList *list, P_Statement *statement)
{
	switch (statement->kind) {
	case P_StatementKind_Expression: {
		P_Expression *expression = statement->data.expression;
		p_expression_print(arena, list, expression);
		break;
	}

	case P_StatementKind_Invalid:
		string_list_push(arena, list, str_lit("<invalid statement>"));
		break;
	}
}

function void
p_entity_print(Arena *arena, StringList *list, P_Entity *entity, isize *tabs)
{
	switch (entity->kind) {
	case P_EntityKind_Procedure: {
		P_Procedure *procedure = &entity->data.procedure;
		string_list_push(arena, list, str_lit("proc "));

		if (procedure->name.data == 0) {
			string_list_push(arena, list, str_lit("<missing>"));
		} else {
			string_list_push(arena, list, procedure->name);
		}

		string_list_push(arena, list, str_lit("() {"));

		if (procedure->body != 0) {
			(*tabs)++;

			for (P_Statement *stmt = procedure->body; stmt != 0; stmt = stmt->next) {
				p_newline_print(arena, list, tabs);
				p_statement_print(arena, list, stmt);
			}

			(*tabs)--;
			p_newline_print(arena, list, tabs);
		}

		string_list_push(arena, list, str_lit("}"));
		break;
	}

	case P_EntityKind_Invalid:
		string_list_push(arena, list, str_lit("<invalid entity>"));
		break;
	}

	p_newline_print(arena, list, tabs);
}

function String
p_parse_result_stringify(Arena *arena, P_ParseResult parse)
{
	Temp temp = temp_begin(&arena, 1);

	StringList list = {0};
	isize tabs = 0;

	for (P_Entity *entity = parse.root.first_entity; entity != 0; entity = entity->next) {
		p_entity_print(arena, &list, entity, &tabs);
	}

	string_list_pushf(temp.arena, &list, "%td errors:\n", parse.diagnostics.count);
	for (D_DiagnosticNode *node = parse.diagnostics.first; node != 0; node = node->next) {
		string_list_push(temp.arena, &list, str_lit("    "));
		d_diagnostic_print(temp.arena, node->diagnostic, &list);
		string_list_push(temp.arena, &list, str_lit("\n"));
	}

	String result = string_list_join(arena, list);
	temp_end(temp);
	return result;
}

function void
p_tests(void)
{
	Temp temp = temp_begin(0, 0);

	String sep = str_lit("===\n");
	b32 update_tests = os_env_get(temp.arena, str_lit("UPDATE_TESTS")).data != 0;

	OS_Entry *entries = os_directory_entries(temp.arena, str_lit("code/parser/test_data"));

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

		String actual_output = p_parse_result_stringify(temp.arena, parse);

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
