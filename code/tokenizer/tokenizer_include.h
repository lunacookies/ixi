typedef u8 TK_TokenKind;
enum {
	TK_TokenKind_Error,
	TK_TokenKind_Identifier,
};

typedef struct TK_Span TK_Span;
struct TK_Span {
	s32 start;
	s32 end;
};

typedef struct TK_Error TK_Error;
struct TK_Error {
	TK_Error *next;
	TK_Error *prev;
	TK_Span span;
	String message;
};

typedef struct TK_TokenizeResult TK_TokenizeResult;
struct TK_TokenizeResult {
	isize token_count;
	TK_TokenKind *kinds;
	TK_Span *spans;

	isize error_count;
	TK_Error *first_error;
	TK_Error *last_error;
};

function void tk_tokenize(Arena *arena, TK_TokenizeResult *result, String source);
function String tk_string_from_token_kind(TK_TokenKind kind);
