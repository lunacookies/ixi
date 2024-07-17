typedef struct SM_Expression SM_Expression;

typedef struct SM_UnaryExpression SM_UnaryExpression;
struct SM_UnaryExpression {
	P_UnaryOperator op;
	SM_Expression *operand;
};

typedef struct SM_BinaryExpression SM_BinaryExpression;
struct SM_BinaryExpression {
	SM_Expression *lhs;
	SM_Expression *rhs;
	P_BinaryOperator op;
};

typedef enum {
	SM_ExpressionKind_Invalid,
	SM_ExpressionKind_Number,
	SM_ExpressionKind_Unary,
	SM_ExpressionKind_Binary,
} SM_ExpressionKind;

typedef union SM_ExpressionData SM_ExpressionData;
union SM_ExpressionData {
	f64 number;
	SM_UnaryExpression unary;
	SM_BinaryExpression binary;
};

struct SM_Expression {
	SM_ExpressionKind kind;
	SM_ExpressionData data;
};

typedef enum {
	SM_StatementKind_Invalid,
	SM_StatementKind_Expression,
} SM_StatementKind;

typedef union SM_StatementData SM_StatementData;
union SM_StatementData {
	SM_Expression *expression;
};

typedef struct SM_Statement SM_Statement;
struct SM_Statement {
	SM_Statement *next;
	SM_StatementKind kind;
	SM_StatementData data;
};

typedef struct SM_Procedure SM_Procedure;
struct SM_Procedure {
	SM_Procedure *next;
	String name;
	SM_Statement *body;
};

typedef struct SM_AnalysisResult SM_AnalysisResult;
struct SM_AnalysisResult {
	SM_Procedure *first_procedure;
	D_DiagnosticList diagnostics;
};

function void sm_analyze(Arena *arena, SM_AnalysisResult *result, P_ParseResult parse);
