/**
 * minisched.h - Minimalist Process Scheduler
 * 
 * A simple educational implementation of a process scheduler
 * using round-robin algorithm.
 */

#ifndef MINISCHED_H
#define MINISCHED_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

/**
 * Priority levels for tasks
 */
typedef enum {
    PRIORITY_LOW,
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
    PRIORITY_REALTIME
} priority_level_t;

/**
 * Task status values
 */
typedef enum {
    TASK_READY,     /* Task is ready to run */
    TASK_RUNNING,   /* Task is currently running */
    TASK_BLOCKED,   /* Task is waiting for a resource */
    TASK_SLEEPING,  /* Task is sleeping for a time interval */
    TASK_FINISHED   /* Task has completed execution */
} task_status_t;

/**
 * Task context - represents execution state
 */
typedef struct {
    void* stack_pointer;
    size_t stack_size;
    /* Other architecture-specific context information would go here */
} task_context_t;

/**
 * Function pointer type for task entry points
 */
typedef void (*task_function_t)(void* arg);

/* Forward declaration for use in task trampoline */
struct task;
/**
 * Task structure - represents a schedulable unit of work
 */
typedef struct task {
    uint32_t id;                  /* Unique task identifier */
    char name[32];                /* Human-readable task name */
    priority_level_t priority;    /* Task priority */
    task_status_t status;         /* Current status */
    task_function_t function;     /* Entry point function */
    void* arg;                    /* Argument to pass to the function */
    
    task_context_t context;       /* Execution context */
    
    uint32_t time_slice;          /* Allocated CPU time slice (in ms) */
    uint32_t time_used;           /* CPU time used in current slice */
    
    struct timespec created_at;   /* When the task was created */
    struct timespec last_run;     /* When the task last ran */
    
    struct task* next;            /* Pointer to next task in queue */
} task_t;

/**
 * Scheduler state - maintains the state of the scheduler
 */
typedef struct {
    task_t* ready_queue;          /* Queue of tasks ready to run */
    task_t* current;              /* Currently executing task */
    uint32_t task_count;          /* Number of tasks in the system */
    bool running;                 /* Whether scheduler is running */
    uint32_t default_time_slice;  /* Default time slice for new tasks (in ms) */
} scheduler_state_t;

/**
 * Initialize the scheduler
 * 
 * This function must be called before any other scheduler functions.
 * 
 * @return true on success, false on failure
 */
bool scheduler_init(void);

/**
 * Create a new task
 * 
 * @param function The entry point function for the task
 * @param priority The priority level for the task
 * @param arg Argument to pass to the task function
 * @param name Optional name for the task (can be NULL)
 * 
 * @return A new task object on success, NULL on failure
 */
task_t* create_task(task_function_t function, priority_level_t priority, 
                   void* arg, const char* name);

/**
 * Add a task to the scheduler
 * 
 * @param task The task to add
 * 
 * @return true on success, false on failure
 */
bool scheduler_add_task(task_t* task);

/**
 * Start running the scheduler
 * 
 * This function will not return until scheduler_shutdown() is called
 * from within a task.
 * 
 * @return true if shutdown was clean, false on error
 */
bool scheduler_run(void);

/**
 * Shutdown the scheduler
 * 
 * This will stop the scheduler and clean up resources.
 * Must be called from within a task.
 */
void scheduler_shutdown(void);

/**
 * Yield execution from the current task
 * 
 * This allows the current task to voluntarily give up its time slice,
 * letting other tasks run.
 */
void task_yield(void);

/**
 * Sleep the current task for the specified number of milliseconds
 * 
 * @param ms Milliseconds to sleep
 */
void task_sleep_ms(uint32_t ms);

#endif /* MINISCHED_H */

