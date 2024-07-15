typedef struct Arena Arena;
struct Arena {
	u8 *buffer;
	isize used;
	isize committed;
	isize reserved;
};

typedef struct Temp Temp;
struct Temp {
	Arena *arena;
	isize pos;
};

enum {
	arena_reserve_size = 128 * 1024 * 1024,
	arena_commit_size = 64 * 1024,
};

function Arena *arena_alloc(void);
function void _arena_clear(Arena *arena) __attribute__((unused));
function void _arena_release(Arena *arena) __attribute__((unused));
function void *arena_push(Arena *arena, isize size, isize align);

function Temp temp_begin(Arena **conflicts, isize conflict_count);
function void temp_end(Temp temp);

#define push_array(arena, type, count)                                                             \
	((type *)arena_push((arena), size_of(type) * (count), align_of(type)))
#define push_struct(arena, type) (push_array(arena, type, 1))
