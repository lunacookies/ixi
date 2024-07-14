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
os_decommit(void *ptr, isize size)
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
