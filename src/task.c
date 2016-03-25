//
//  task.cpp
//  dummy
//
//  Created by Steve Tranby on 3/2/16.
//  Copyright © 2016 Steve Tranby. All rights reserved.
//

//#include <stdint.h>
#include <system.h>

// TODO: alloc on heap instead (no variable)
internal Task* runningTask;
i32 getpid() { return runningTask->pid; }

#define MAX_PROCESSES 10

// TODO: confirm zeroes out, but also should use kmalloc instead of hardcode
internal Task processes[MAX_PROCESSES];
internal u8 processesCount = 0;
internal u8 curProcessIndex = 0;

//////////////////////////////////////////////////////////////////
// Testing

static void main_A()
{
    trace("Starting Process A!\n");
    for(;;)
    {
        //trace("Running Process A!\n");
        //delay_ms(100);
        k_preempt();
    }
}

static void main_B()
{
    trace("Starting Process B!\n");
    for(;;)
    {
        //trace("Running Process B!\n");
        //delay_ms(200);
        k_preempt();
    }
}

static void main_C()
{
    trace("Starting Process C!\n");
    for(;;)
    {
        //trace("Running Process C!\n");
        //delay_ms(300);
        k_preempt();
    }
}

void k_preempt()
{
    //trace("ticks: %d\n", timer_ticks());

    Task* nextTask = NULL;
    for(int i=0; i<MAX_PROCESSES; ++i)
    {
        ++curProcessIndex;
        curProcessIndex = curProcessIndex % MAX_PROCESSES;
        //trace("checking if proc index %d is ready\n", curProcessIndex);
        if(nextTask->isActive && nextTask != runningTask)
        {
            Task* last = runningTask;
            Task* next = runningTask->next;
            runningTask = next ? next : &processes[0];

//            trace("BEFORE (%d) switchTask [%d : %p : %x : %x] to [%d : %p : %x : %x]\n",
//                  curProcessIndex,
//                  last->pid, last, last->regs.cr3, last->regs.eip,
//                  runningTask->pid, runningTask, runningTask->regs.cr3, runningTask->regs.eip);

            switchTask(&last->regs, &runningTask->regs);

//            trace("AFTER (%d) switchTask [%d : %p : %x : %x] to [%d : %p : %x : %x]\n",
//                  curProcessIndex,
//                  last->pid, last, last->regs.cr3, last->regs.eip,
//                  runningTask->pid, runningTask, runningTask->regs.cr3, runningTask->regs.eip);

            return;
        }
    }
}

//////////////////////////////////////////////////////////////////

void initTasking()
{
    trace("init multitasking\n");

    for(int i=0; i<MAX_PROCESSES; ++i) {
        processes[i].isActive = false;
        processes[i].pid = i;
    }

    Task* mainTask;
    mainTask = &processes[0];
    mainTask->isActive = true;
    processesCount = 1;

    // Get EFLAGS and CR3
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;"
                 : "=m"(mainTask->regs.cr3)
                 :
                 : "%eax");

    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;"
                 : "=m"(mainTask->regs.eflags)
                 :
                 : "%eax");

    trace("main.cr3 = %x\n", mainTask->regs.cr3);
    trace("main.eflags = %b\n", mainTask->regs.eflags);

    //createTask(&otherTask, otherMain, mainTask.regs.eflags, (u32*)mainTask.regs.cr3);
    // TODO: probably want different page directory, even if identical
    createTask(&processes[1], main_A, mainTask->regs.eflags, (u32*)mainTask->regs.cr3);
    createTask(&processes[2], main_B, mainTask->regs.eflags, (u32*)mainTask->regs.cr3);
    createTask(&processes[3], main_C, mainTask->regs.eflags, (u32*)mainTask->regs.cr3);

//    mainTask.next = &otherTask;
//    otherTask.next = &mainTask;

    runningTask = &processes[0];
}

u32 allocPage()
{
    return 0;
}

void createTask(Task* task, TaskHandler handler, u32 flags, u32* pagedir)
{
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (u32) handler;
    task->regs.cr3 = (u32) pagedir;

    //task->regs.esp = (u32) allocPage() + 0x1000; // Not implemented here
    // need to make sure we don't overlap with other processes or we'll destroy stacks
    task->regs.esp = (u32) allocPage() + 0x1000 * processesCount;

    task->next = 0;
    task->isActive = true;

    processes[processesCount-1].next = task;
    ++processesCount;
}

void removeTask(Task* task)
{
    --processesCount;
}
