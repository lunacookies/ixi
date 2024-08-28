function String
string_clone(Arena *arena, String string)
{
	String result = {0};
	result.length = string.length;
	result.data = push_array(arena, u8, result.length);
	memory_copy(result.data, string.data, result.length);
	return result;
}

function b32
string_equal(String s1, String s2)
{
	if (s1.length != s2.length) {
		return 0;
	}

	return memory_compare(s1.data, s2.data, s1.length) == 0;
}

function String
string_slice(String string, isize start, isize end)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(start <= string.length);
	assert(end <= string.length);
	assert(start <= end);

	String result = {0};
	result.data = string.data + start;
	result.length = end - start;
	return result;
}

function isize
string_find(String string, String sub)
{
	u8 *start = memory_find(string.data, string.length, sub.data, sub.length);
	if (start == 0) {
		return -1;
	}

	isize result = start - string.data;
	assert(result >= 0);
	return result;
}

function b32
string_cut(String string, String *before, String *after, String sep)
{
	isize sep_position = string_find(string, sep);
	if (sep_position == -1) {
		*before = string;
		memory_zero_struct(after);
		return 0;
	}

	*before = string_slice(string, 0, sep_position);
	*after = string_slice(string, sep_position + sep.length, string.length);
	return 1;
}

function f64
f64_from_string(String string)
{
	Temp temp = temp_begin(0, 0);
	f64 result = strtof(cstring_from_string(temp.arena, string), 0);
	temp_end(temp);
	return result;
}

function isize
cstring_length(char *cstring)
{
	return (isize)strlen(cstring);
}

function char *
cstring_from_string(Arena *arena, String string)
{
	char *result = push_array(arena, char, string.length + 1);
	memory_copy(result, string.data, string.length);
	return result;
}

function String
push_stringfv(Arena *arena, char *fmt, va_list ap)
{
	va_list ap2;
	va_copy(ap2, ap);

	String result = {0};
	result.length = vsnprintf(0, 0, fmt, ap);
	result.data = push_array(arena, u8, result.length + 1);
	isize bytes_written = vsnprintf((char *)result.data, result.length + 1, fmt, ap2) + 1;
	assert(bytes_written == result.length + 1);

	va_end(ap2);
	return result;
}

function String
push_stringf(Arena *arena, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	String result = push_stringfv(arena, fmt, ap);
	va_end(ap);
	return result;
}

function void
string_list_push_no_copy(Arena *arena, StringList *list, String string)
{
	StringNode *node = push_struct(arena, StringNode);
	node->string = string;

	if (list->first == 0) {
		assert(list->last == 0);
		list->first = node;
		list->last = node;
	} else {
		assert(list->last != 0);
		list->last->next = node;
		list->last = node;
	}

	list->length += string.length;
	list->count++;
}

function void
string_list_push(Arena *arena, StringList *list, String string)
{
	String dup = {0};
	dup.length = string.length;
	dup.data = push_array(arena, u8, dup.length);
	memory_copy(dup.data, string.data, dup.length);
	string_list_push_no_copy(arena, list, dup);
}

function void
string_list_pushf(Arena *arena, StringList *list, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	String string = push_stringfv(arena, fmt, ap);
	string_list_push_no_copy(arena, list, string);
	va_end(ap);
}

function String
string_list_join(Arena *arena, StringList list)
{
	String result = {0};
	result.length = list.length;
	result.data = push_array(arena, u8, result.length);

	isize i = 0;
	for (StringNode *node = list.first; node != 0; node = node->next) {
		memory_copy(result.data + i, node->string.data, node->string.length);
		i += node->string.length;
	}

	assert(i == list.length);
	return result;
}
