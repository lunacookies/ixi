typedef struct P_Expression P_Expression;

typedef enum {
	P_UnaryOperator_Invalid,
	P_UnaryOperator_Identity,
	P_UnaryOperator_Negate,
	P_UnaryOperator_BitComplement,
} P_UnaryOperator;

typedef struct P_UnaryExpression P_UnaryExpression;
struct P_UnaryExpression {
	P_UnaryOperator op;
	P_Expression *operand;
};

typedef enum {
	P_BinaryOperator_Invalid,
	P_BinaryOperator_Add,
	P_BinaryOperator_Subtract,
	P_BinaryOperator_Multiply,
	P_BinaryOperator_Divide,
	P_BinaryOperator_Modulo,
	P_BinaryOperator_BitAnd,
	P_BinaryOperator_BitOr,
	P_BinaryOperator_BitXor,
	P_BinaryOperator_ShiftLeft,
	P_BinaryOperator_ShiftRight,
	P_BinaryOperator_Equal,
	P_BinaryOperator_NotEqual,
	P_BinaryOperator_LessThan,
	P_BinaryOperator_GreaterThan,
	P_BinaryOperator_LessThanEqual,
	P_BinaryOperator_GreaterThanEqual,
	P_BinaryOperator_And,
	P_BinaryOperator_Or,
} P_BinaryOperator;

typedef struct P_BinaryExpression P_BinaryExpression;
struct P_BinaryExpression {
	P_Expression *lhs;
	P_Expression *rhs;
	P_BinaryOperator op;
};

typedef enum {
	P_ExpressionKind_Invalid,
	P_ExpressionKind_Number,
	P_ExpressionKind_Unary,
	P_ExpressionKind_Binary,
} P_ExpressionKind;

typedef union P_ExpressionData P_ExpressionData;
union P_ExpressionData {
	f64 number;
	P_UnaryExpression unary;
	P_BinaryExpression binary;
};

struct P_Expression {
	P_ExpressionKind kind;
	P_ExpressionData data;
	D_Span span;
};

typedef enum {
	P_StatementKind_Invalid,
	P_StatementKind_Expression,
} P_StatementKind;

typedef union P_StatementData P_StatementData;
union P_StatementData {
	P_Expression *expression;
};

typedef struct P_Statement P_Statement;
struct P_Statement {
	P_Statement *next;
	P_StatementKind kind;
	P_StatementData data;
	D_Span span;
};

typedef struct P_Procedure P_Procedure;
struct P_Procedure {
	String name;
	P_Statement *body;
};

typedef enum {
	P_EntityKind_Invalid,
	P_EntityKind_Procedure,
} P_EntityKind;

typedef union P_EntityData P_EntityData;
union P_EntityData {
	P_Procedure procedure;
};

typedef struct P_Entity P_Entity;
struct P_Entity {
	P_Entity *next;
	P_EntityKind kind;
	P_EntityData data;
};

typedef struct P_Root P_Root;
struct P_Root {
	P_Entity *first_entity;
};

typedef struct P_ParseResult P_ParseResult;
struct P_ParseResult {
	P_Root root;
	D_DiagnosticList diagnostics;
};

function void p_parse(Arena *arena, P_ParseResult *result, String source);
