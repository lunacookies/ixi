static isize
align_pad_pow_2(isize n, isize align)
{
	assert(n >= 0);
	assert(align > 0);
	assert(__builtin_popcountll((usize)align) == 1);

	isize mask = align - 1;
	return -n & mask;
}
