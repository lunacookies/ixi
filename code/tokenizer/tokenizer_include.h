#define tk_list_of_token_kinds \
	X(Error) \
	X(Identifier) \
	X(Number) \
	X(ProcKw) \
	X(StructKw) \
	X(ConstKw) \
	X(VarKw) \
	X(IfKw) \
	X(ElseKw) \
	X(ForKw) \
	X(BreakKw) \
	X(ContinueKw) \
	X(SwitchKw) \
	X(CaseKw) \
	X(ReturnKw) \
	X(Bang) \
	X(Hash) \
	X(Percent) \
	X(Ampersand) \
	X(LParen) \
	X(RParen) \
	X(Asterisk) \
	X(Plus) \
	X(Comma) \
	X(Hyphen) \
	X(Period) \
	X(Slash) \
	X(Colon) \
	X(Semi) \
	X(LAngle) \
	X(Equal) \
	X(RAngle) \
	X(LSquare) \
	X(RSquare) \
	X(Caret) \
	X(LBrace) \
	X(Pipe) \
	X(RBrace) \
	X(Tilde) \
	X(LAngleEqual) \
	X(RAngleEqual) \
	X(Pipe2) \
	X(Ampersand2) \
	X(Equal2) \
	X(BangEqual) \
	X(LAngle2) \
	X(RAngle2)

#define X(token_kind) TK_TokenKind_##token_kind,
typedef enum { tk_list_of_token_kinds } TK_TokenKind;
#undef X

#define X(token_kind) str_lit(#token_kind),
global const String tk_token_kind_names[] = {tk_list_of_token_kinds};
#undef X

typedef struct TK_TokenizeResult TK_TokenizeResult;
struct TK_TokenizeResult {
	isize token_count;
	TK_TokenKind *kinds;
	D_Span *spans;
	D_DiagnosticList diagnostics;
};

function void tk_tokenize(Arena *arena, TK_TokenizeResult *result, String source);
function void tk_tests(void);
