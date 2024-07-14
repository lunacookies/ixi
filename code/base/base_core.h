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

function isize align_pad_pow_2(isize n, isize align);

#define breakpoint() (__builtin_debugtrap())
#define assert(condition)                                                                          \
	if (!(condition))                                                                          \
		breakpoint();
