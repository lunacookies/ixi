function Arena *
arena_alloc(void)
{
	u8 *region = os_reserve(arena_reserve_size);
	os_commit(region, arena_commit_size);

	Arena *arena = (Arena *)region;
	arena->buffer = region;
	arena->used = size_of(Arena);
	arena->committed = arena_commit_size;
	arena->reserved = arena_reserve_size;
	return arena;
}

function void
arena_reset_to_pos(Arena *arena, isize pos)
{
	assert(pos >= size_of(Arena));
	assert(pos <= arena->used);

	memory_zero(arena->buffer + pos, arena->used - pos);
	arena->used = pos;
}

function void
_arena_clear(Arena *arena)
{
	arena_reset_to_pos(arena, size_of(Arena));
}

function void
_arena_release(Arena *arena)
{
	os_release(arena->buffer, arena->reserved);
}

function void *
arena_push(Arena *arena, isize size, isize align)
{
	assert(size >= 0);
	assert(align > 0);

	isize padding = align_pad_pow_2(arena->used, align);
	isize needed_space = size + padding;

	isize remaining_reserved_space = arena->reserved - arena->used;
	assert(needed_space <= remaining_reserved_space);

	isize remaining_committed_space = arena->committed - arena->used;
	if (remaining_committed_space < needed_space) {
		isize needed_commit_count =
		        (needed_space + arena_commit_size - 1) / arena_commit_size;
		isize needed_commit_bytes = needed_commit_count * arena_commit_size;
		assert(needed_commit_bytes <= remaining_reserved_space);
		os_commit(arena->buffer + arena->committed, needed_commit_bytes);
		arena->committed += needed_commit_bytes;
	}

	void *ptr = arena->buffer + arena->used + padding;
	arena->used += needed_space;
	return ptr;
}

typedef struct ThreadContext ThreadContext;
struct ThreadContext {
	b32 initialized;
	Arena *arenas[2];
};

_Thread_local ThreadContext thread_context;

function Temp
temp_begin(Arena **conflicts, isize conflict_count)
{
	assert(conflict_count >= 0);

	if (!thread_context.initialized) {
		for (isize i = 0; i < array_count(thread_context.arenas); i++) {
			thread_context.arenas[i] = arena_alloc();
		}
		thread_context.initialized = 1;
	}

	Temp result = {0};

	for (isize arena_index = 0; arena_index < array_count(thread_context.arenas);
	        arena_index++) {
		Arena *arena = thread_context.arenas[arena_index];
		b32 any_conflicts_match = 0;

		for (isize conflict_index = 0; conflict_index < conflict_count; conflict_index++) {
			Arena *conflict = conflicts[conflict_index];
			if (conflict == arena) {
				any_conflicts_match = 1;
			}
		}

		if (!any_conflicts_match) {
			result.arena = arena;
			break;
		}
	}

	assert(result.arena != 0);

	result.pos = result.arena->used;
	return result;
}

function void
temp_end(Temp temp)
{
	arena_reset_to_pos(temp.arena, temp.pos);
}
