#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/list.h>


unsigned long **sys_call_table;

struct ancestry {
  pid_t ancestor[10];
  pid_t siblings[100];
  pid_t children[100];
};


// Sourced /usr/src/linux/include/linux/syscalls.h
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, struct ancestry *response);
asmlinkage long (*ref_sys_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(unsigned int fd);
asmlinkage long (*ref_sys_read)(unsigned int fd, char __user *buf, size_t count);


asmlinkage long new_sys_cs3013_syscall1(void) {
  printk(KERN_INFO "\"'Hello world?!' More like 'Goodbye, world!' EXTERMINATE!\" -- Dalek\n");
  printk(KERN_INFO "Now runing original syscall1");
  ref_sys_cs3013_syscall1();
  return 0;
}

asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, struct ancestry *response) {
  unsigned short ktarget_pid;
  struct ancestry *kancestry = kmalloc(sizeof (struct ancestry), GFP_KERNEL);
  struct task_struct *t = kmalloc(sizeof (struct task_struct), GFP_KERNEL);
  struct list_head *list;
  struct task_struct *cur_child = kmalloc(sizeof (struct task_struct), GFP_KERNEL);
  printk(KERN_INFO "p2Who dare invoke me!\n");
  if (copy_from_user(&ktarget_pid, target_pid, sizeof(ktarget_pid)) || copy_from_user(kancestry, response, sizeof(struct ancestry))) {
    printk(KERN_INFO "p2Err?\n");
    return -1;
  } else {
    printk(KERN_INFO "p2it's fine, ktarget_pid is %hu\n", ktarget_pid);
  }
  t = pid_task(find_vpid(ktarget_pid), PIDTYPE_PID);
  printk(KERN_INFO "%hu has the following children:\n", ktarget_pid);
  int childIndex = 0;
  list_for_each(list, &t->children) {
    if (childIndex <= 99) {
      cur_child = list_entry(list, struct task_struct, sibling);
      kancestry->children[childIndex] = cur_child->pid;
      childIndex++;
      printk(KERN_INFO "%s[%d]\n", cur_child->comm, cur_child->pid);
    } else {
      printk(KERN_INFO "skipping %s[%d] as over 100 child limit\n", cur_child->comm, cur_child->pid);
    }
  }
  printk(KERN_INFO "printing siblings:\n");
  int siblingIndex = 0;
  struct task_struct *cur_sib = kmalloc(sizeof (struct task_struct), GFP_KERNEL);
  list_for_each(list, &t->sibling) {
    if (siblingIndex <= 99) {
      cur_sib = list_entry(list, struct task_struct, sibling);
      kancestry->siblings[siblingIndex] = cur_sib->pid;
      siblingIndex++;
      printk(KERN_INFO "%s[%d]", cur_sib->comm, cur_sib->pid);
    } else {
      printk(KERN_INFO "skipping %s[%d] as over 100 sib limit\n", cur_sib->comm, cur_sib->pid);
    }
  }
  if (copy_to_user(response, kancestry, sizeof (struct ancestry))) {
    return -1; // err
  }
  return 0;
}

asmlinkage long new_sys_open(const char __user *filename, int flags, umode_t mode) {
  long ret;
  kuid_t uid;
  uid = current_uid();
  // Only log to syslog if uid is over 1000, because the first user account uid starts at 1000
  if (uid.val >= 1000) printk(KERN_INFO "User %u is opening file: %s\n", uid.val, filename);
  ret = ref_sys_open(filename, flags, mode);
  return ret;
}

asmlinkage long new_sys_close(unsigned int fd) {
  long ret;
  kuid_t uid;
  uid = current_uid();
  // Only log to syslog if uid is over 1000, because the first user account uid starts at 1000
  //printk(KERN_INFO "hi\n");
  if (uid.val >= 1000) printk(KERN_INFO "User %u is closing file descriptor: %u\n", uid.val, fd);
  ret = ref_sys_close(fd);
  return ret;
}

asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count) {
  long ret;
  kuid_t uid;
  uid = current_uid();
  // Only log to syslog if uid is over 1000, because the first user account uid starts at 1000
  if (strstr(buf, "VIRUS") != NULL) printk(KERN_INFO "User %u read from file descriptor %u, but that read contained malicious code!\n", uid.val, fd);
  ret = ref_sys_read(fd, buf, count);
  return ret;
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX\n",
	     (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  // Source function names from /usr/src/linux/arch/x86/entry/syscalls/syscall_32.tbl
  ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
  ref_sys_open = (void *)sys_call_table[__NR_open];
  ref_sys_close = (void *)sys_call_table[__NR_close];
  ref_sys_read = (void *)sys_call_table[__NR_read];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
  sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
  sys_call_table[__NR_close] = (unsigned long *)new_sys_close;
  sys_call_table[__NR_read] = (unsigned long *)new_sys_read;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
  sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
  sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
