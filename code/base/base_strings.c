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
_push_stringf(Arena *arena, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	String result = push_stringfv(arena, fmt, ap);
	va_end(ap);
	return result;
}
