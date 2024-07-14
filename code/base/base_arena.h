typedef struct Arena Arena;
struct Arena {
	u8 *buffer;
	isize used;
	isize committed;
	isize reserved;
};

enum {
	arena_reserve_size = 128 * 1024 * 1024,
	arena_commit_size = 64 * 1024,
};

function Arena *arena_alloc(void);
function void arena_clear(Arena *arena);
function void arena_release(Arena *arena);
function void *arena_push(Arena *arena, isize size, isize align);

#define push_struct(arena, type) ((type *)arena_push((arena), sizeof(type), alignof(type)))
