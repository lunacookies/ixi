function void
cg_push(Arena *arena, StringList *out)
{
	string_list_pushf(arena, out, "\tsub\tsp, sp, #16\n");
	string_list_pushf(arena, out, "\tstr\tx8, [sp]\n");
}

function void
cg_pop(Arena *arena, StringList *out, String destination_register)
{
	string_list_pushf(arena, out, "\tldr\t%.*s, [sp]\n", str_fmt(destination_register));
	string_list_pushf(arena, out, "\tadd\tsp, sp, #16\n");
}

function void
cg_gen_expression(Arena *arena, StringList *out, SM_Expression *expression)
{
	switch (expression->kind) {
	case SM_ExpressionKind_Number:
		string_list_pushf(arena, out, "\tmov\tx8, #%.f\n", expression->number);
		break;

	case SM_ExpressionKind_Unary:
		cg_gen_expression(arena, out, expression->unary.operand);
		switch (expression->unary.op) {
		case P_UnaryOperator_Negate:
			string_list_pushf(arena, out, "\tneg\tx8, x8\n");
			break;

		case P_UnaryOperator_BitComplement:
			string_list_pushf(arena, out, "\tmvn\tx8, x8\n");
			break;

		case P_UnaryOperator_Identity:
		case P_UnaryOperator_Invalid: assert(0); break;
		}
		break;

	case SM_ExpressionKind_Binary:
		cg_gen_expression(arena, out, expression->binary.lhs);
		cg_push(arena, out);
		cg_gen_expression(arena, out, expression->binary.rhs);
		cg_pop(arena, out, str_lit("x9"));
		switch (expression->binary.op) {
		case P_BinaryOperator_Add:
			string_list_pushf(arena, out, "\tadd\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_Subtract:
			string_list_pushf(arena, out, "\tsub\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_Multiply:
			string_list_pushf(arena, out, "\tmul\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_Divide:
			string_list_pushf(arena, out, "\tsdiv\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_Modulo:
			string_list_pushf(arena, out, "\tsdiv\tx10, x8, x9\n");
			string_list_pushf(arena, out, "\tmsub\tx8, x10, x9, x8\n");
			break;

		case P_BinaryOperator_BitAnd:
			string_list_pushf(arena, out, "\tand\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_BitOr:
			string_list_pushf(arena, out, "\torr\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_BitXor:
			string_list_pushf(arena, out, "\teor\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_ShiftLeft:
			string_list_pushf(arena, out, "\tlsl\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_ShiftRight:
			string_list_pushf(arena, out, "\tasr\tx8, x8, x9\n");
			break;

		case P_BinaryOperator_Equal:
			string_list_pushf(arena, out, "\tcmp\tx8, x9\n");
			string_list_pushf(arena, out, "\tcset\tx8, eq\n");
			break;

		case P_BinaryOperator_NotEqual:
			string_list_pushf(arena, out, "\tcmp\tx8, x9\n");
			string_list_pushf(arena, out, "\tcset\tx8, ne\n");
			break;

		case P_BinaryOperator_LessThan:
			string_list_pushf(arena, out, "\tcmp\tx8, x9\n");
			string_list_pushf(arena, out, "\tcset\tx8, lt\n");
			break;

		case P_BinaryOperator_GreaterThan:
			string_list_pushf(arena, out, "\tcmp\tx8, x9\n");
			string_list_pushf(arena, out, "\tcset\tx8, gt\n");
			break;

		case P_BinaryOperator_LessThanEqual:
			string_list_pushf(arena, out, "\tcmp\tx8, x9\n");
			string_list_pushf(arena, out, "\tcset\tx8, le\n");
			break;

		case P_BinaryOperator_GreaterThanEqual:
			string_list_pushf(arena, out, "\tcmp\tx8, x9\n");
			string_list_pushf(arena, out, "\tcset\tx8, ge\n");
			break;

		case P_BinaryOperator_And:
		case P_BinaryOperator_Or:
		case P_BinaryOperator_Invalid: assert(0); break;
		}
		break;

	case SM_ExpressionKind_Invalid: assert(0); break;
	}
}

function void
cg_gen_statement(Arena *arena, StringList *out, SM_Statement *statement)
{
	switch (statement->kind) {
	case SM_StatementKind_Expression:
		cg_gen_expression(arena, out, statement->expression);
		break;

	case SM_StatementKind_Invalid: assert(0); break;
	}
}

function void
cg_gen_procedure(Arena *arena, StringList *out, SM_Procedure *procedure)
{
	string_list_pushf(arena, out, ".global _%.*s\n", str_fmt(procedure->name));
	string_list_pushf(arena, out, ".align 2\n");
	string_list_pushf(arena, out, "_%.*s:\n", str_fmt(procedure->name));

	for (SM_Statement *statement = procedure->body; statement != 0;
	        statement = statement->next) {
		cg_gen_statement(arena, out, procedure->body);
	}

	string_list_pushf(arena, out, "\tmov\tx0, x8\n");
	string_list_pushf(arena, out, "\tret\n");
}

function void
cg_gen(SM_Procedure *first_procedure, String executable_path)
{
	Temp temp = temp_begin(0, 0);
	StringList assembly_list = {0};

	for (SM_Procedure *procedure = first_procedure; procedure != 0;
	        procedure = procedure->next) {
		cg_gen_procedure(temp.arena, &assembly_list, procedure);
	}

	String assembly = string_list_join(temp.arena, assembly_list);

	String assembly_path = push_stringf(temp.arena, "%.*s.s", str_fmt(executable_path));
	String object_path = push_stringf(temp.arena, "%.*s.o", str_fmt(executable_path));
	os_write_file(assembly_path, assembly);

	{
		StringList invocation = {0};
		string_list_push(temp.arena, &invocation, str_lit("as"));
		string_list_push(temp.arena, &invocation, str_lit("-o"));
		string_list_push(temp.arena, &invocation, object_path);
		string_list_push(temp.arena, &invocation, assembly_path);
		os_execute_command(invocation);
	}

	{
		StringList invocation = {0};
		string_list_push(temp.arena, &invocation, str_lit("ld"));
		string_list_push(temp.arena, &invocation, str_lit("-o"));
		string_list_push(temp.arena, &invocation, executable_path);
		string_list_push(temp.arena, &invocation, object_path);
		os_execute_command(invocation);
	}

	temp_end(temp);
}
