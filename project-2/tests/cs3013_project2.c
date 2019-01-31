#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

unsigned long **sys_call_table;

// Sourced /usr/src/linux/include/linux/syscalls.h
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(unsigned int fd);
asmlinkage long (*ref_sys_read)(unsigned int fd, char __user *buf, size_t count);


asmlinkage long new_sys_cs3013_syscall1(void) {
  printk(KERN_INFO "\"'Hello world?!' More like 'Goodbye, world!' EXTERMINATE!\" -- Dalek\n");
  printk(KERN_INFO "Now runing original syscall1");
  ref_sys_cs3013_syscall1();
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

int contains_virus(char *buf){
  int i;
  int contains;
  int end;
  char *check = "VIRUS";
  end = 0;
  while(end < 4){
    if(buf[end] == '\0') return 0;
    ++end;
  }
  while(buf[end] != '\0'){
    contains = 1;
    for(i = 0; i < 5; i++){
      if(buf[end-i] != check[4-i]) contains = 0;
    }
    if(contains == 1) return 1;
    ++end;
  }
  return 0;
}

asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count) {
  long ret;
  kuid_t uid;
  uid = current_uid();
  // Only log to syslog if uid is over 1000, because the first user account uid starts at 1000
  //if (contains_virus(buf)) printk(KERN_INFO "User %u read from file descriptor %u and it contains virus!\n", uid.val, fd);
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
  ref_sys_open = (void *)sys_call_table[__NR_open];
  ref_sys_close = (void *)sys_call_table[__NR_close];
  ref_sys_read = (void *)sys_call_table[__NR_read];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
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
  sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
  sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
  sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
