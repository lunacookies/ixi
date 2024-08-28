function void *os_reserve(isize size);
function void os_commit(void *ptr, isize size);
function void os__decommit(void *ptr, isize size) __attribute__((unused));
function void os_release(void *ptr, isize size);

typedef struct OS_Entry OS_Entry;
struct OS_Entry {
	OS_Entry *next;
	String name;
	String path;
	b32 is_directory;
};

function isize os_file_size(String path);
function String os_read_file(Arena *arena, String path);
function void os_write_file(String path, String contents);
function OS_Entry *os_directory_entries(Arena *arena, String path);

function String os_env_get(Arena *arena, String name);

typedef struct OS_CommandExecution OS_CommandExecution;
struct OS_CommandExecution {
	s8 exit_code;
};

function OS_CommandExecution os_execute_command(StringList invocation);
