function void *
os_reserve(isize size)
{
	assert(size > 0);

	mach_port_t task = mach_task_self();
	void *ptr = 0;

	kern_return_t kr =
	        vm_allocate(task, (vm_address_t *)&ptr, (vm_size_t)size, VM_FLAGS_ANYWHERE);
	assert(kr == KERN_SUCCESS);

	kr = vm_protect(task, (vm_address_t)ptr, (vm_size_t)size, 0, VM_PROT_NONE);
	assert(kr == KERN_SUCCESS);

	return ptr;
}

function void
os_commit(void *ptr, isize size)
{
	assert(size > 0);

	mach_port_t task = mach_task_self();

	vm_prot_t prot = VM_PROT_READ | VM_PROT_WRITE;
	kern_return_t kr = vm_protect(task, (vm_address_t)ptr, (vm_size_t)size, 0, prot);
	assert(kr == KERN_SUCCESS);
}

function void
os__decommit(void *ptr, isize size)
{
	assert(size > 0);

	mach_port_t task = mach_task_self();
	kern_return_t kr = vm_protect(task, (vm_address_t)ptr, (vm_size_t)size, 0, VM_PROT_NONE);
	assert(kr == KERN_SUCCESS);
	kr = vm_behavior_set(task, (vm_address_t)ptr, (vm_size_t)size, VM_BEHAVIOR_REUSABLE);
	assert(kr == KERN_SUCCESS);
}

function void
os_release(void *ptr, isize size)
{
	assert(size > 0);

	mach_port_t task = mach_task_self();
	kern_return_t kr = vm_deallocate(task, (vm_address_t)ptr, (vm_size_t)size);
	assert(kr == KERN_SUCCESS);
}

function isize
os_file_size(String path)
{
	Temp temp = temp_begin(0, 0);

	struct stat s = {0};
	stat(cstring_from_string(temp.arena, path), &s);

	temp_end(temp);
	return s.st_size;
}

function String
os_read_file(Arena *arena, String path)
{
	Temp temp = temp_begin(&arena, 1);

	isize size = os_file_size(path);

	String result = {0};
	result.length = size;
	result.data = push_array(arena, u8, size);

	s32 fd = open(cstring_from_string(temp.arena, path), O_RDONLY);
	read(fd, result.data, (usize)size);

	temp_end(temp);
	return result;
}

function OS_Entry *
os_directory_entries(Arena *arena, String path)
{
	Temp temp = temp_begin(&arena, 1);

	OS_Entry *first_entry = 0;
	OS_Entry *last_entry = 0;

	DIR *dirp = opendir(cstring_from_string(temp.arena, path));

	for (;;) {
		struct dirent *unix_entry = readdir(dirp);
		if (unix_entry == 0) {
			break;
		}

		OS_Entry *entry = push_struct(arena, OS_Entry);
		if (first_entry == 0) {
			assert(last_entry == 0);
			first_entry = entry;
			last_entry = entry;
		} else {
			assert(last_entry != 0);
			last_entry->next = entry;
			last_entry = entry;
		}

		entry->name.length = unix_entry->d_namlen;
		entry->name.data = push_array(arena, u8, entry->name.length);
		memory_copy(entry->name.data, &unix_entry->d_name, entry->name.length);

		entry->path = push_stringf(arena, "%.*s/%.*s", str_fmt(path), str_fmt(entry->name));

		entry->is_directory = unix_entry->d_type == DT_DIR;
	}

	temp_end(temp);
	return first_entry;
}
