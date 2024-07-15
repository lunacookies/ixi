function void
memory_copy(void *dst, void *src, isize n)
{
	assert(n >= 0);
	memmove(dst, src, n);
}

function void
memory_zero(void *dst, isize n)
{
	assert(n >= 0);
	memset(dst, 0, n);
}

function isize
memory_compare(void *p1, void *p2, isize n)
{
	assert(n >= 0);
	return memcmp(p1, p2, (usize)n);
}

function void *
memory_find(void *haystack, isize haystack_size, void *needle, isize needle_size)
{
	assert(haystack_size >= 0);
	assert(needle_size >= 0);
	return memmem(haystack, (usize)haystack_size, needle, (usize)needle_size);
}

function isize
align_pad_pow_2(isize n, isize align)
{
	assert(n >= 0);
	assert(align > 0);
	assert(__builtin_popcountll((usize)align) == 1);

	isize mask = align - 1;
	return -n & mask;
}
