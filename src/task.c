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
internal Task mainTask;
internal Task otherTask;

internal Task* runningTask;
i32 getpid() { return runningTask->pid; }

int processes_count = 0;

//////////////////////////////////////////////////////////////////
// Testing

static void otherMain()
{
    // Not implemented here...?
    trace("Hello multitasking world!\n");
    k_preempt();
}

void k_preempt()
{
    trace("ticks: %d\n", timer_ticks());
    Task* lastTask = runningTask;
    runningTask = runningTask->next;
    trace("mainTask = %p, otherTask = %p, runningTask = %p, lastTask = %p\n",
          &mainTask, &otherTask, runningTask, lastTask);
    switchTask(&lastTask->regs, &runningTask->regs);
}

//////////////////////////////////////////////////////////////////

void initTasking()
{
    trace("init multitasking\n");
    
    // Get EFLAGS and CR3
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;"
                 : "=m"(mainTask.regs.cr3)
                 :
                 : "%eax");

    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;"
                 : "=m"(mainTask.regs.eflags)
                 :
                 : "%eax");

    trace("main.cr3 = %x\n", mainTask.regs.cr3);
    trace("main.eflags = %b\n", mainTask.regs.eflags);

    processes_count = 1;

    createTask(&otherTask, otherMain, mainTask.regs.eflags, (u32*)mainTask.regs.cr3);

    mainTask.next = &otherTask;
    otherTask.next = &mainTask;
    runningTask = &mainTask;
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
    task->regs.esp = (u32) allocPage() + 0x1000 * processes_count;

    task->next = 0;
    task->isActive = true;

    ++processes_count;
}

void removeTask(Task* task)
{
    --processes_count;
}
