function Arena *
arena_alloc(void)
{
	u8 *region = os_reserve(arena_reserve_size);
	os_commit(region, arena_commit_size);

	Arena *arena = (Arena *)region;
	arena->buffer = region;
	arena->committed = arena_commit_size;
	arena->reserved = arena_reserve_size;
	return arena;
}

function void
arena_clear(Arena *arena)
{
	arena->used = 0;
}

function void
arena_release(Arena *arena)
{
	os_release(arena->buffer, arena->reserved);
}

function void *
arena_push(Arena *arena, isize size, isize align)
{
	assert(size > 0);
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
