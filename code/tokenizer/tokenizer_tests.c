function String
tk_tokenize_result_stringify(Arena *arena, TK_TokenizeResult tokenize, String source)
{
	Temp temp = temp_begin(&arena, 1);

	StringList list = {0};
	string_list_pushf(temp.arena, &list, "%td bytes\n", source.length);

	string_list_pushf(temp.arena, &list, "%td tokens:\n", tokenize.token_count);
	for (isize i = 0; i < tokenize.token_count; i++) {
		TK_TokenKind kind = tokenize.kinds[i];
		D_Span span = tokenize.spans[i];
		String kind_string = tk_token_kind_names[kind];
		String token_text = string_slice(source, span.start, span.end);

		string_list_pushf(temp.arena, &list, "    %.*s@%d..%d \"%.*s\"\n",
		        str_fmt(kind_string), span.start, span.end, str_fmt(token_text));
	}

	string_list_pushf(temp.arena, &list, "%td errors:\n", tokenize.diagnostics.count);
	for (D_DiagnosticNode *node = tokenize.diagnostics.first; node != 0; node = node->next) {
		string_list_push(temp.arena, &list, str_lit("    "));
		d_diagnostic_print(temp.arena, node->diagnostic, &list);
		string_list_push(temp.arena, &list, str_lit("\n"));
	}

	String result = string_list_join(arena, list);
	temp_end(temp);
	return result;
}

function void
tk_tests(void)
{
	Temp temp = temp_begin(0, 0);

	String sep = str_lit("===\n");
	b32 update_tests = os_env_get(temp.arena, str_lit("UPDATE_TESTS")).data != 0;

	OS_Entry *entries = os_directory_entries(temp.arena, str_lit("code/tokenizer/test_data"));

	for (OS_Entry *entry = entries; entry != 0; entry = entry->next) {
		if (entry->is_directory) {
			continue;
		}

		String contents = os_read_file(temp.arena, entry->path);

		String source = {0};
		String expected_output = {0};
		b32 found = string_cut(contents, &source, &expected_output, sep);
		assert(found);

		TK_TokenizeResult tokenize = {0};
		tk_tokenize(temp.arena, &tokenize, source);

		String actual_output = tk_tokenize_result_stringify(temp.arena, tokenize, source);

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
