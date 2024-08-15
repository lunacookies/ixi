#define tk_list_of_token_kinds \
	X(EOF, "end of file") \
	X(Error, "erroneous token") \
	X(Identifier, "identifier") \
	X(Number, "number") \
	X(ProcKw, "“proc”") \
	X(StructKw, "“struct”") \
	X(ConstKw, "“const”") \
	X(VarKw, "“var”") \
	X(IfKw, "“if”") \
	X(ElseKw, "“else”") \
	X(ForKw, "“for”") \
	X(BreakKw, "“break”") \
	X(ContinueKw, "“continue”") \
	X(SwitchKw, "“switch”") \
	X(CaseKw, "“case”") \
	X(ReturnKw, "“return”") \
	X(Bang, "“!”") \
	X(Hash, "“#”") \
	X(Percent, "“%”") \
	X(Ampersand, "“&”") \
	X(LParen, "“(”") \
	X(RParen, "“)”") \
	X(Asterisk, "“*”") \
	X(Plus, "“+”") \
	X(Comma, "“,”") \
	X(Hyphen, "“-”") \
	X(Period, "“.”") \
	X(Slash, "“/”") \
	X(Colon, "“:”") \
	X(Semi, "“;”") \
	X(LAngle, "“<”") \
	X(Equal, "“=”") \
	X(RAngle, "“>”") \
	X(LSquare, "“[”") \
	X(RSquare, "“]”") \
	X(Caret, "“^”") \
	X(LBrace, "“{”") \
	X(Pipe, "“|”") \
	X(RBrace, "“}”") \
	X(Tilde, "“~”") \
	X(LAngleEqual, "“<=”") \
	X(RAngleEqual, "“>=”") \
	X(Pipe2, "“||”") \
	X(Ampersand2, "“&&”") \
	X(Equal2, "“==”") \
	X(BangEqual, "“!=”") \
	X(LAngle2, "“<<”") \
	X(RAngle2, "“>>”") \
	X(Arrow, "->")

#define X(name, human_name) TK_TokenKind_##name,
typedef enum { tk_list_of_token_kinds } TK_TokenKind;
#undef X

#define X(name, human_name) str_lit(#name),
global read_only String tk_token_kind_names[] = {tk_list_of_token_kinds};
#undef X

#define X(name, human_name) str_lit(human_name),
global read_only String tk_token_kind_human_names[] = {tk_list_of_token_kinds};
#undef X

typedef struct TK_TokenizeResult TK_TokenizeResult;
struct TK_TokenizeResult {
	isize token_count;
	TK_TokenKind *kinds;
	D_Span *spans;
	D_DiagnosticList diagnostics;
};

function void tk_tokenize(Arena *arena, TK_TokenizeResult *result, String source);
