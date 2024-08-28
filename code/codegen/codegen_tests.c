function void
cg_tests(void)
{
	Temp temp = temp_begin(0, 0);

	String sep = str_lit("===\n");
	b32 update_tests = os_env_get(temp.arena, str_lit("UPDATE_TESTS")).data != 0;

	OS_Entry *entries = os_directory_entries(temp.arena, str_lit("code/codegen/test_data"));

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

		String executable_path = str_lit("build/test");
		cg_gen(analysis.first_procedure, executable_path);

		StringList invocation = {0};
		string_list_push(temp.arena, &invocation, executable_path);
		OS_CommandExecution execution = os_execute_command(invocation);

		String actual_output =
		        push_stringf(temp.arena, "exit code: %d\n", execution.exit_code);

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
