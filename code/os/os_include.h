function void *os_reserve(isize size);
function void os_commit(void *ptr, isize size);
function void __attribute__((unused)) os__decommit(void *ptr, isize size);
function void os_release(void *ptr, isize size);
