typedef struct D_Span D_Span;
struct D_Span {
	s32 start;
	s32 end;
};

typedef enum {
	D_Severity_Invalid,
	D_Severity_Error,
} D_Severity;

typedef struct D_Diagnostic D_Diagnostic;
struct D_Diagnostic {
	D_Span span;
	D_Severity severity;
	String message;
};

typedef struct D_DiagnosticNode D_DiagnosticNode;
struct D_DiagnosticNode {
	D_DiagnosticNode *next;
	D_Diagnostic diagnostic;
};

typedef struct D_DiagnosticList D_DiagnosticList;
struct D_DiagnosticList {
	isize count;
	D_DiagnosticNode *first;
	D_DiagnosticNode *last;
};

function D_Diagnostic *d_diagnostic_list_push(Arena *arena, D_DiagnosticList *list);
function void d_diagnostic_print(Arena *arena, D_Diagnostic diagnostic, StringList *list);
