typedef enum {
	TK_TokenKind_Error,

	TK_TokenKind_Identifier,
	TK_TokenKind_Number,

	TK_TokenKind_ProcKw,
	TK_TokenKind_StructKw,
	TK_TokenKind_ConstKw,
	TK_TokenKind_VarKw,
	TK_TokenKind_IfKw,
	TK_TokenKind_ElseKw,
	TK_TokenKind_ForKw,
	TK_TokenKind_BreakKw,
	TK_TokenKind_ContinueKw,
	TK_TokenKind_SwitchKw,
	TK_TokenKind_CaseKw,
	TK_TokenKind_ReturnKw,

	TK_TokenKind_Bang,
	TK_TokenKind_Hash,
	TK_TokenKind_Percent,
	TK_TokenKind_Ampersand,
	TK_TokenKind_LParen,
	TK_TokenKind_RParen,
	TK_TokenKind_Asterisk,
	TK_TokenKind_Plus,
	TK_TokenKind_Comma,
	TK_TokenKind_Hyphen,
	TK_TokenKind_Period,
	TK_TokenKind_Slash,
	TK_TokenKind_Colon,
	TK_TokenKind_Semi,
	TK_TokenKind_LAngle,
	TK_TokenKind_Equal,
	TK_TokenKind_RAngle,
	TK_TokenKind_LSquare,
	TK_TokenKind_RSquare,
	TK_TokenKind_Caret,
	TK_TokenKind_LBrace,
	TK_TokenKind_Pipe,
	TK_TokenKind_RBrace,
	TK_TokenKind_Tilde,

	TK_TokenKind_LAngleEqual,
	TK_TokenKind_RAngleEqual,
	TK_TokenKind_Pipe2,
	TK_TokenKind_Ampersand2,
	TK_TokenKind_Equal2,
	TK_TokenKind_BangEqual,
	TK_TokenKind_LAngle2,
	TK_TokenKind_RAngle2,
} TK_TokenKind;

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

function void tk_tests(void);
