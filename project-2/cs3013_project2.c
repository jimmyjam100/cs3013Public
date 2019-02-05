#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/list.h>
//#include <linux/types.h>
//#include <linux/signal.h>


unsigned long **sys_call_table;

struct ancestry {
  pid_t ancestors[10];
  pid_t siblings[100];
  pid_t children[100];
};


// Sourced /usr/src/linux/include/linux/syscalls.h
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, struct ancestry *response);
asmlinkage long (*ref_sys_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(unsigned int fd);
asmlinkage long (*ref_sys_read)(unsigned int fd, char __user *buf, size_t count);

//replace syscall1 with new text
asmlinkage long new_sys_cs3013_syscall1(void) {
  printk(KERN_INFO "\"'Hello world?!' More like 'Goodbye, world!' EXTERMINATE!\" -- Dalek\n");
  printk(KERN_INFO "Now runing original syscall1");
  ref_sys_cs3013_syscall1();
  return 0;
}

//a function that recursivly goes through parents and stores them to a ancestry pointer
void fill_with_ancestor(struct ancestry *anc, struct task_struct *cur, int i) {
  if (i > 9 || cur == NULL || (i != 0 && cur->pid == anc->ancestors[i-1])) { //if already have 10 or parent is repeating do nothing
    return;
  }
  printk(KERN_INFO "%s[%d]\n", cur->comm, cur->pid); //otherwise print out that it was added and add it to the list and call recursivly
  anc->ancestors[i] = cur->pid;
  fill_with_ancestor(anc, cur->parent, i + 1);
}

//the function that will replace syscall2 for phase 2
asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, struct ancestry *response) {
  unsigned short ktarget_pid;
  struct ancestry *kancestry = kmalloc(sizeof (struct ancestry), GFP_KERNEL); //alocate space for the ancestry we will copy info into and change
  struct task_struct *t = kmalloc(sizeof (struct task_struct), GFP_KERNEL); //allocate space for the struct that will contain all the info we want
  struct list_head *list;
  struct task_struct *cur_child = kmalloc(sizeof (struct task_struct), GFP_KERNEL); 
  //printk(KERN_INFO "Who dare invoke me!\n"); //let the user know that the program is being run
  if (copy_from_user(&ktarget_pid, target_pid, sizeof(ktarget_pid)) || copy_from_user(kancestry, response, sizeof(struct ancestry))/* || kill(ktarget_pid, 0) == -1*/) { //check to make sure the input is correct
    printk(KERN_INFO "Err?\n"); // print out an error and return an error if it is not
    kfree(kancestry);
    kfree(t);
    kfree(list);
    return -1;
  } else { //if everything is good print out the pid that is being used
    //printk(KERN_INFO "it's fine, ktarget_pid is %hu\n", ktarget_pid);
  }
  t = pid_task(find_vpid(ktarget_pid), PIDTYPE_PID); //get all the info needed in the form of a task_struct which was recommended for the project
  if(t == NULL){ //if the pid was not valid return an error
    printk(KERN_INFO "pid was not vaild\n");
    kfree(kancestry);
    kfree(t);
    kfree(list);
    return -1;
  }
  printk(KERN_INFO "%hu has the following children:\n", ktarget_pid); //let the user know that the next outputs are children
  int childIndex = 0; //set the index for storing the information to the beggining
  list_for_each(list, &t->children) { //for each child
    if (childIndex <= 99) { //if there is still room in the array print it out and add it to the array
      cur_child = list_entry(list, struct task_struct, sibling);
      kancestry->children[childIndex] = cur_child->pid;
      childIndex++;
      printk(KERN_INFO "%s[%d]\n", cur_child->comm, cur_child->pid);
    } else { //otherwise let the user know you are skipping it
      printk(KERN_INFO "skipping %s[%d] as over 100 child limit\n", cur_child->comm, cur_child->pid);
    }
  }
  printk(KERN_INFO "printing siblings:\n"); //let the user know that the next outputs are siblings
  int siblingIndex = 0; //set the index for stroing the information to the beggining
  struct task_struct *cur_sib = kmalloc(sizeof (struct task_struct), GFP_KERNEL);
  list_for_each(list, &t->sibling) { //for each sibling
    if (siblingIndex <= 99) { //if there is still room in the array print it out and add it to the array
      cur_sib = list_entry(list, struct task_struct, sibling);
      kancestry->siblings[siblingIndex] = cur_sib->pid;
      siblingIndex++;
      printk(KERN_INFO "%s[%d]", cur_sib->comm, cur_sib->pid);
    } else { //otherwise let the user know you are skipping it
      printk(KERN_INFO "skipping %s[%d] as over 100 sib limit\n", cur_sib->comm, cur_sib->pid);
    }
  }
  printk(KERN_INFO ":: Printing Ancestors ::\n"); //let the user know that the next outputs are parents
  fill_with_ancestor(kancestry, t->parent, 0); //run the function that recursivly goes through parents printing them and adding them to the array
  if (copy_to_user(response, kancestry, sizeof (struct ancestry))) { //copy data back to the user space struct
    kfree(kancestry);
    kfree(t);
    kfree(list);
    return -1; // err
  }
  kfree(kancestry);
  kfree(t);
  kfree(list);
  return 0;
}

//the function that replaces sys_open
asmlinkage long new_sys_open(const char __user *filename, int flags, umode_t mode) {
  long ret;
  kuid_t uid;
  uid = current_uid();
  // Only log to syslog if uid is over 1000, because the first user account uid starts at 1000
  if (uid.val >= 1000) printk(KERN_INFO "User %u is opening file: %s\n", uid.val, filename);
  ret = ref_sys_open(filename, flags, mode);
  return ret;
}

//the function that replaces sys_close
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

//the function that replaces sys_read
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
