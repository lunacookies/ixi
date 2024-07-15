#define function static
#define global static
#define local_persist static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef ptrdiff_t isize;
typedef size_t usize;

typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

#define size_of(type) ((isize)sizeof(type))
#define align_of(type) ((isize)alignof(type))

#define array_count(array) (size_of(array) / size_of((array)[0]))

function void memory_copy(void *dst, void *src, isize n);
#define memory_copy_array(dst, src, n) (memory_copy((dst), (src), size_of((dst)[0]) * (n)))
#define memory_copy_struct(dst, src) (memory_copy_array((dst), (src), 1))

function void memory_zero(void *dst, isize n);
#define memory_zero_struct(dst) (memory_zero((dst), size_of(dst)))

function isize memory_compare(void *p1, void *p2, isize n);
function void *memory_find(void *haystack, isize haystack_size, void *needle, isize needle_size);

function isize align_pad_pow_2(isize n, isize align);

#define breakpoint() (__builtin_debugtrap())
#define assert(condition)                                                                          \
	if (!(condition))                                                                          \
		breakpoint();
