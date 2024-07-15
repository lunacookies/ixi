typedef struct String String;
struct String {
	u8 *data;
	isize length;
};

#define str_lit(s) ((String){.data = (u8 *)(s), .length = size_of(s) - 1})
#define str_fmt(s) (int)(s).length, (s).data

function String push_stringfv(Arena *arena, char *fmt, va_list ap);
function __attribute__((unused)) String _push_stringf(Arena *arena, char *fmt, ...);
