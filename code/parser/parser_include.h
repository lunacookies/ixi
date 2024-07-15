typedef enum {
	P_EntityKind_Procedure,
} P_EntityKind;

typedef struct P_Procedure P_Procedure;
struct P_Procedure {
	String name;
};

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
	P_Entity *last_entity;
};

typedef struct P_ParseResult P_ParseResult;
struct P_ParseResult {
	P_Root root;
	D_DiagnosticList diagnostics;
};

function void p_parse(Arena *arena, P_ParseResult *result, String source);
function void p_tests(void);
