//
//  task.cpp
//  dummy
//
//  Created by Steve Tranby on 3/2/16.
//  Copyright © 2016 Steve Tranby. All rights reserved.
//

//#include <stdint.h>
#include <system.h>

internal Task* runningTask;
internal Task* mainTask;
internal Task* readyQueue;

// Some externs are needed to access members in paging.c...
extern page_directory_entry* kernel_directory;
extern page_directory* current_directory;

extern void alloc_frame(page_table_entry*,int,int);
extern u32 initial_esp;
extern u32 read_eip();

#define MAX_PROCESSES 10
// TODO: confirm zeroes out, but also should use kmalloc instead of hardcode
internal Task processes[MAX_PROCESSES];
internal u8 processes_count = 0;
internal u8 curProcessIndex = 0;

//////////////////////////////////////////////////////////////////
// Testing

static void main_A()
{
    trace("Starting Process A!\n");
    for(;;)
    {
        trace("Running Process A!\n");
        delay_ms(100);
        //k_preempt();
    }
}

static void main_B()
{
    trace("Starting Process B!\n");
    for(;;)
    {
        trace("Running Process B!\n");
        delay_ms(200);
        //k_preempt();
    }
}

static void main_C()
{
    trace("Starting Process C!\n");
    for(;;)
    {
        trace("Running Process C!\n");
        delay_ms(300);
        //k_preempt();
    }
}

void k_preempt_kernel()
{
    trace("k_preempt_kernel called [%d : %d]\n", timer_ticks(), runningTask->pid);

    // probably should disable interrupts
    Task* nextTask = NULL;
    for(int i=0; i<MAX_PROCESSES; ++i) {
        ++curProcessIndex;
        curProcessIndex = curProcessIndex % MAX_PROCESSES;
        trace("checking if proc index %d is ready\n", curProcessIndex);
        nextTask = &processes[curProcessIndex];
        if(nextTask->isActive && nextTask != runningTask) {
            Task* last = runningTask;
            runningTask = nextTask;

//            trace("BEFORE (%d) switchTask [%d : %p : %x : %x] to [%d : %p : %x : %x]\n",
//                  curProcessIndex,
//                  last->pid, last, last->regs.cr3, last->regs.eip,
//                  runningTask->pid, runningTask, runningTask->regs.cr3, runningTask->regs.eip);
//
//            switchTaskInterrupt(&last->regs, &runningTask->regs);
//
//            // NOTE: this probably won't ever get called
//            trace("AFTER (%d) switchTask [%d : %p : %x : %x] to [%d : %p : %x : %x]\n",
//                  curProcessIndex,
//                  last->pid, last, last->regs.cr3, last->regs.eip,
//                  runningTask->pid, runningTask, runningTask->regs.cr3, runningTask->regs.eip);

            return;
        }
    }
}

void k_preempt()
{
    trace("k_preempt called [%d : %d]\n", timer_ticks(), runningTask->pid);

    // probably should disable interrupts
    Task* nextTask = NULL;
    for(int i=0; i<MAX_PROCESSES; ++i) {
        ++curProcessIndex;
        curProcessIndex = curProcessIndex % MAX_PROCESSES;
        trace("checking if proc index %d is ready\n", curProcessIndex);
        nextTask = &processes[curProcessIndex];
        if(nextTask->isActive && nextTask != runningTask) {
            Task* last = runningTask;
            runningTask = nextTask;

//            trace("BEFORE (%d) switchTask [%d : %p : %x : %x] to [%d : %p : %x : %x]\n",
//                  curProcessIndex,
//                  last->pid, last, last->regs.cr3, last->regs.eip,
//                  runningTask->pid, runningTask, runningTask->regs.cr3, runningTask->regs.eip);
//
//            switchTask(&last->regs, &runningTask->regs);
//
//            trace("AFTER (%d) switchTask [%d : %p : %x : %x] to [%d : %p : %x : %x]\n",
//                  curProcessIndex,
//                  last->pid, last, last->regs.cr3, last->regs.eip,
//                  runningTask->pid, runningTask, runningTask->regs.cr3, runningTask->regs.eip);

            return;
        }
    }
}

//////////////////////////////////////////////////////////////////
// Scheduler


void k_schedule_process(int pid) {
    // add to list of tasks, find open slot
}

void k_unschedule_process(int pid) {
    // remove process state and release resources
}

//////////////////////////////////////////////////////////////////
// Tasking

//void initTasking()
//{
//    trace("init multitasking\n");
//
//    for(int i=0; i<MAX_PROCESSES; ++i) {
//        processes[i].isActive = false;
//        processes[i].pid = i;
//    }
//
//    mainTask = &processes[0];
//    mainTask->isActive = true;
//    processes_count = 1;
//
//    // Get EFLAGS and CR3
//    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;"
//                 : "=m"(mainTask->regs.cr3)
//                 :
//                 : "%eax");
//
//    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;"
//                 : "=m"(mainTask->regs.eflags)
//                 :
//                 : "%eax");
//
//    trace("main.cr3 = %x\n", mainTask->regs.cr3);
//    trace("main.eflags = %b\n", mainTask->regs.eflags);
//
//    runningTask = mainTask;
//
//    // TODO: probably want different page directory, even if identical
//    createTask(&processes[1], main_A, mainTask->regs.eflags, (u32*)mainTask->regs.cr3);
//    createTask(&processes[2], main_B, mainTask->regs.eflags, (u32*)mainTask->regs.cr3);
//    createTask(&processes[3], main_C, mainTask->regs.eflags, (u32*)mainTask->regs.cr3);
//}
//
//u32 allocPage()
//{
//    trace("ERROR: not implemented");
//    return 0;
//}
//
//void createTask(Task* task, TaskHandler handler, u32 flags, u32* pagedir)
//{
//    task->regs.eax = 0;
//    task->regs.ebx = 0;
//    task->regs.ecx = 0;
//    task->regs.edx = 0;
//    task->regs.esi = 0;
//    task->regs.edi = 0;
//    task->regs.eflags = flags;
//    task->regs.eip = (u32) handler;
//    task->regs.cr3 = (u32) pagedir;
//    // need to make sure we don't overlap with other processes or we'll destroy stacks
//    task->regs.esp = (u32) allocPage() + 0x1000 * processes_count;
//    task->next = NULL;
//    task->isActive = true;
//    ++processes_count;
//}
//
//void removeTask(Task* task)
//{
//    --processes_count;
//}

void initialise_tasking()
{
    // Rather important stuff happening, no interrupts please!
    asm volatile("cli");

    // Relocate the stack so we know where it is.
    move_stack((void*)0xE0000000, 0x2000);

    // Initialise the first task (kernel task)
    runningTask = readyQueue = (Task*)kmalloc(sizeof(Task));
    runningTask->pid = curProcessIndex++;
    runningTask->esp = runningTask->ebp = 0;
    runningTask->eip = 0;
    runningTask->pageDirectory = current_directory;
    runningTask->next = 0;
    runningTask->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);

    // Reenable interrupts.
    asm volatile("sti");
}

void move_stack(void* new_stack_start, u32 size)
{
    u32 i;
    // Allocate some space for the new stack.
    for( i = (u32)new_stack_start;
        i >= ((u32)new_stack_start - size);
        i -= 0x1000)
    {
        // General-purpose stack is in user-mode.
        alloc_frame( get_page(i, 1, current_directory), 0 /* User mode */, 1 /* Is writable */ );
    }

    trace("allocated new stack\n");

    // Flush the TLB by reading and writing the page directory address again.
    u32 pd_addr;
    asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
    asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

    trace("line: %d\n", __LINE__);

    // Old ESP and EBP, read from registers.
    u32 old_stack_pointer; asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
    u32 old_base_pointer;  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer));

    // Offset to add to old stack addresses to get a new stack address.
    u32 offset            = (u32)new_stack_start - initial_esp;

    // New ESP and EBP.
    u32 new_stack_pointer = old_stack_pointer + offset;
    u32 new_base_pointer  = old_base_pointer  + offset;

    trace("line: %d\n", __LINE__);

    // Copy the stack.
    kmemcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp-old_stack_pointer);

    trace("line: %d\n", __LINE__);

    // Backtrace through the original stack, copying new values into
    // the new stack.
    for(i = (u32)new_stack_start; i > (u32)new_stack_start-size; i -= 4)
    {
        u32 tmp = * (u32*)i;
        // If the value of tmp is inside the range of the old stack, assume it is a base pointer
        // and remap it. This will unfortunately remap ANY value in this range, whether they are
        // base pointers or not.
        if (( old_stack_pointer < tmp) && (tmp < initial_esp))
        {
            tmp = tmp + offset;
            u32 *tmp2 = (u32*)i;
            *tmp2 = tmp;
        }
    }

    trace("line: %d, esp = %x, ebp = %x\n", __LINE__, new_stack_pointer, new_base_pointer);

    // Change stacks.
    asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
    asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));

    trace("line: %d\n", __LINE__);
}

void switch_task()
{
    // If we haven't initialised tasking yet, just return.
    if (!runningTask)
        return;

    // Read esp, ebp now for saving later on.
    u32 esp, ebp, eip;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    // Read the instruction pointer. We do some cunning logic here:
    // One of two things could have happened when this function exits -
    //   (a) We called the function and it returned the EIP as requested.
    //   (b) We have just switched tasks, and because the saved EIP is essentially
    //       the instruction after read_eip(), it will seem as if read_eip has just
    //       returned.
    // In the second case we need to return immediately. To detect it we put a dummy
    // value in EAX further down at the end of this function. As C returns values in EAX,
    // it will look like the return value is this dummy value! (0x12345).
    eip = read_eip();

    // Have we just switched tasks?
    if (eip == 0x12345)
        return;

    // No, we didn't switch tasks. Let's save some register values and switch.
    runningTask->eip = eip;
    runningTask->esp = esp;
    runningTask->ebp = ebp;

    // Get the next task to run.
    runningTask = runningTask->next;
    // If we fell off the end of the linked list start again at the beginning.
    if (!runningTask) runningTask = readyQueue;

    eip = runningTask->eip;
    esp = runningTask->esp;
    ebp = runningTask->ebp;

    // Make sure the memory manager knows we've changed page directory.
    current_directory = runningTask->pageDirectory;

    // Change our kernel stack over.
    set_kernel_stack(runningTask->kernel_stack+KERNEL_STACK_SIZE);
    
    // Here we:
    // * Stop interrupts so we don't get interrupted.
    // * Temporarily put the new EIP location in ECX.
    // * Load the stack and base pointers from the new task struct.
    // * Change page directory to the physical address (physicalAddr) of the new directory.
    // * Put a dummy value (0x12345) in EAX so that above we can recognise that we've just
    //   switched task.
    // * Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
    //   the next instruction.
    // * Jump to the location in ECX (remember we put the new EIP in there).
    asm volatile("         \
                 cli;                 \
                 mov %0, %%ecx;       \
                 mov %1, %%esp;       \
                 mov %2, %%ebp;       \
                 mov %3, %%cr3;       \
                 mov $0x12345, %%eax; \
                 sti;                 \
                 jmp *%%ecx           "
                 : : "r"(eip), "r"(esp), "r"(ebp), "r"(current_directory->physicalAddress));
}

i32 fork()
{
    // We are modifying kernel structures, and so cannot be interrupted.
    asm volatile("cli");

    // Take a pointer to this process' task struct for later reference.
    Task* parent_task = (Task*)runningTask;

    // Clone the address space.
    page_directory* directory = clone_directory(current_directory);

    // Create a new process.
    Task *new_task = (Task*)kmalloc(sizeof(Task));
    new_task->pid = curProcessIndex++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->pageDirectory = directory;
    runningTask->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);
    new_task->next = 0;

    // Add it to the end of the ready queue.
    // Find the end of the ready queue...
    Task *tmp_task = (Task*)readyQueue;
    while (tmp_task->next)
        tmp_task = tmp_task->next;
    // ...And extend it.
    tmp_task->next = new_task;

    // This will be the entry point for the new process.
    u32 eip = read_eip();

    // We could be the parent or the child here - check.
    if (runningTask == parent_task)
    {
        // We are the parent, so set up the esp/ebp/eip for our child.
        u32 esp; asm volatile("mov %%esp, %0" : "=r"(esp));
        u32 ebp; asm volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        // All finished: Reenable interrupts.
        asm volatile("sti");

        // And by convention return the PID of the child.
        return new_task->pid;
    }
    else
    {
        // We are the child - by convention return 0.
        return 0;
    }

}

i32 getpid()
{
    return runningTask->pid;
}

void switch_to_user_mode()
{
    // Set up our kernel stack.
    set_kernel_stack(runningTask->kernel_stack+KERNEL_STACK_SIZE);

    // Set up a stack structure for switching to user mode.
    asm volatile("  \
                 cli; \
                 mov $0x23, %ax; \
                 mov %ax, %ds; \
                 mov %ax, %es; \
                 mov %ax, %fs; \
                 mov %ax, %gs; \
                 \
                 \
                 mov %esp, %eax; \
                 pushl $0x23; \
                 pushl %esp; \
                 pushf; \
                 pushl $0x1B; \
                 push $1f; \
                 iret; \
                 1: \
                 "); 
    
}
