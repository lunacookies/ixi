function D_Diagnostic *
d_diagnostic_list_push(Arena *arena, D_DiagnosticList *list)
{
	D_DiagnosticNode *node = push_struct(arena, D_DiagnosticNode);

	if (list->first == 0) {
		assert(list->last == 0);
		list->first = node;
		list->last = node;
	} else {
		assert(list->last != 0);
		list->last->next = node;
		list->last = node;
	}

	list->count++;
	return &node->diagnostic;
}

function void
d_diagnostic_print(Arena *arena, D_Diagnostic diagnostic, StringList *list)
{
	switch (diagnostic.severity) {
	case D_Severity_Error: string_list_push(arena, list, str_lit("error")); break;

	case D_Severity_Invalid:
		string_list_push(arena, list, str_lit("<unknown severity>"));
		break;
	}

	string_list_pushf(arena, list, " at %d..%d: %.*s", diagnostic.span.start,
	        diagnostic.span.end, str_fmt(diagnostic.message));
}
