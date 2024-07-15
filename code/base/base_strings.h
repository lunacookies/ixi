typedef struct String String;
struct String {
	u8 *data;
	isize length;
};

#define str_lit(s) ((String){.data = (u8 *)(s), .length = size_of(s) - 1})
#define str_fmt(s) (int)(s).length, (s).data

function b32 string_equal(String s1, String s2);
function String string_slice(String string, isize start, isize end);
function isize string_find(String string, String sub);
function b32 string_cut(String string, String *before, String *after, String sep);

function isize cstring_length(char *cstring);
function char *cstring_from_string(Arena *arena, String string);

function String push_stringfv(Arena *arena, char *fmt, va_list ap);
function String push_stringf(Arena *arena, char *fmt, ...) __attribute__((format(printf, 2, 3)));

typedef struct StringNode StringNode;
struct StringNode {
	StringNode *next;
	String string;
};

typedef struct StringList StringList;
struct StringList {
	StringNode *first;
	StringNode *last;
	isize length;
};

function void string_list_push(Arena *arena, StringList *list, String string);
function void string_list_pushf(Arena *arena, StringList *list, char *fmt, ...)
        __attribute__((format(printf, 3, 4)));
function String string_list_join(Arena *arena, StringList list);
