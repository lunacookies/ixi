function void
memory_copy(void *dst, void *src, isize n)
{
	assert(n > 0);
	memmove(dst, src, n);
}

function void
memory_zero(void *dst, isize n)
{
	assert(n >= 0);
	memset(dst, 0, n);
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
