#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "scheduler.h"

#include <assert.h>
#include <curses.h>
#include <ucontext.h>
#include <time.h>

#include "util.h"

// This is an upper limit on the number of tasks we can create.
#define MAX_TASKS 128

// This is the size of each task's stack memory
#define STACK_SIZE 65536

// States
#define SLEEP 0
#define WAIT 1
#define READ 2
#define RUNNING 3
#define READY 4
#define FINISHED 5

// This struct will hold the all the necessary information for each task
typedef struct task_info {
  // This field stores all the state required to switch back to this task
  ucontext_t context;
  
  int status; //status of current task
  int sleep_until; // time for sleep in ms
  int task_wait; // task that the current task is waiting on

  // This field stores another context. This one is only used when the task
  // is exiting.
  ucontext_t exit_context;
  
  /**
   * TODO: Add fields here so you can:
   * a. Keep track of this task's state.
   * b. If the task is sleeping, when should it wake up?
   * c. If the task is waiting for another task, which task is it waiting for?
   * d. Was the task blocked waiting for user input? Once you successfully
   *   read input, you will need to save it here so it can be returned.
  */
} task_info_t;

int previous_task = 0;
int current_task = 0; //< The handle of the currently-executing task
int num_tasks = 1;    //< The number of tasks created so far
int started_switching = 0; // Have we done our first context switch?
task_info_t tasks[MAX_TASKS]; //< Information for every task
int inpt;

/**
 * Initialize the scheduler. Programs should call this before calling any other
 * functiosn in this file.
 */
void scheduler_init() {
  ucontext_t mainContext;
  getcontext(&mainContext);
  ucontext_t mainExitContext;
  getcontext(&mainExitContext);
  task_info_t mainTask = { mainContext, 3, 0, 0, mainExitContext };
  tasks[0] = mainTask;
  
}

int get_time_ms(){
  return (int)((double)clock())/((double)CLOCKS_PER_SEC)*1000;
}

void findnexttask() {
  while(true) {
    current_task = (current_task + 1) % num_tasks;
    if (tasks[current_task].status == READY) return;
    if (tasks[current_task].status == FINISHED) continue;
    switch (tasks[current_task].status) {
      case WAIT :
        if (tasks[tasks[current_task].task_wait].status == FINISHED) {
          return;
        }
        break;
      case SLEEP :
        if (tasks[current_task].sleep_until <= get_time_ms()){
          return;
        } else {
          break;
        }
      case READ :
        inpt = getch();
        if (inpt == ERR){
          break;
        } else {
          return;
        }
    } // end of switch
  } // end of while
}

void scheduler() {
  previous_task = current_task; // we're in between tasks
  findnexttask();
  swapcontext(&tasks[previous_task].context, &tasks[current_task].context);
}

/**
 * This function will execute when a task's function returns. This allows you
 * to update scheduler states and start another task. This function is run
 * because of how the contexts are set up in the task_create function.
 */
void task_exit() {

  // TODO: Handle the end of a task's execution here
  tasks[current_task].status = FINISHED;
  scheduler();
}

/**
 * Create a new task and add it to the scheduler.
 *
 * \param handle  The handle for this task will be written to this location.
 * \param fn      The new task will run this function.
 */
void task_create(task_t* handle, task_fn_t fn) {
  // Claim an index for the new task
  int index = num_tasks;
  num_tasks++;
  
  // Set the task handle to this index, since task_t is just an int
  *handle = index;
  tasks[index].status = READY;
  // We're going to make two contexts: one to run the task, and one that
  // runs at the end of the task so we can clean up. Start with the second
  
  // First, duplicate the current context as a starting point
  getcontext(&tasks[index].exit_context);
  
  // Set up a stack for the exit context
  tasks[index].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].exit_context.uc_stack.ss_size = STACK_SIZE;
  
  // Set up a context to run when the task function returns. This should call task_exit.
  makecontext(&tasks[index].exit_context, task_exit, 0);
  
  // Now we start with the task's actual running context
  getcontext(&tasks[index].context);
  
  // Allocate a stack for the new task and add it to the context
  tasks[index].context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].context.uc_stack.ss_size = STACK_SIZE;
  
  // Now set the uc_link field, which sets things up so our task will go to the exit context when the task function finishes
  tasks[index].context.uc_link = &tasks[index].exit_context;
  
  // And finally, set up the context to execute the task function
  makecontext(&tasks[index].context, fn, 0);
}

/**
 * Wait for a task to finish. If the task has not yet finished, the scheduler should
 * suspend this task and wake it up later when the task specified by handle has exited.
 *
 * \param handle  This is the handle produced by task_create
 */
void task_wait(task_t handle) {
  // TODO: Block this task until the specified task has exited.
  if (started_switching == 0) {
    started_switching = 1;
    getcontext(&tasks[0].context);
    tasks[0].status = WAIT;
    tasks[0].task_wait = handle;
    scheduler();
  } else {
    tasks[current_task].task_wait = handle;
    scheduler();
    return;
  }
}

/**
 * The currently-executing task should sleep for a specified time. If that time is larger
 * than zero, the scheduler should suspend this task and run a different task until at least
 * ms milliseconds have elapsed.
 * 
 * \param ms  The number of milliseconds the task should sleep.
 */
void task_sleep(size_t ms) {
  // TODO: Block this task until the requested time has elapsed.
  // Hint: Record the time the task should wake up instead of the time left for it to sleep. The bookkeeping is easier this way.
  if (started_switching == 0) {
    started_switching = 1;
    getcontext(&tasks[0].context);
    tasks[0].status = SLEEP;
    tasks[0].sleep_until = ms + get_time_ms();
    scheduler();
  } else {
    tasks[current_task].status = SLEEP;
    tasks[current_task].sleep_until = ms + get_time_ms();
    scheduler();
  }

}

/**
 * Read a character from user input. If no input is available, the task should
 * block until input becomes available. The scheduler should run a different
 * task while this task is blocked.
 *
 * \returns The read character code
 */
int task_readchar() {
  // TODO: Block this task until there is input available.
  // To check for input, call getch(). If it returns ERR, no input was available.
  // Otherwise, getch() will returns the character code that was read.
  if (started_switching == 0) {
    started_switching = 1;
    getcontext(&tasks[0].context);
    tasks[0].status = READ;
  } else {
    tasks[current_task].status = READ;
  }
  scheduler();
  return inpt;
}
