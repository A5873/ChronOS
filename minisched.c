/**
 * minisched.c - Implementation of the Minimalist Process Scheduler
 * 
 * A simple educational implementation of a process scheduler
 * using round-robin algorithm.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#include "minisched.h"

/* Size of each task's stack in bytes */
#define STACK_SIZE (64 * 1024)  /* 64 KB */

/* Default time slice for tasks in milliseconds */
#define DEFAULT_TIME_SLICE 100

/* Global state of the scheduler */
static scheduler_state_t g_scheduler = {0};

/* Task ID counter for generating unique IDs */
static uint32_t next_task_id = 1;

/* Timer for preemptive scheduling */
static struct itimerval timer;

/* Signal handler for timer */
static void timer_handler(int signum);

/**
 * Task trampoline function
 * 
 * This function wraps the actual task function and performs cleanup
 * when the task function returns.
 */
static void task_trampoline(task_t* task) {
    if (!task) return;
    
    /* Execute the task function */
    if (task->function) {
        task->function(task->arg);
    }
    
    /* Task has completed, mark it as finished */
    task->status = TASK_FINISHED;
    printf("Task %s (ID: %u) has finished\n", task->name, task->id);
    
    /* 
     * Don't call task_yield() here as it would return to this point
     * Instead, directly return to let the scheduler handle task completion
     */
    
    /* Never reach here since we return after marking task as finished */
}

/**
 * Initialize a new task's context
 * 
 * Sets up the stack and initial execution context for a task.
 * This is a simplified implementation for educational purposes.
 * 
 * @param task The task to initialize context for
 * @return true on success, false on failure
 */
static bool init_task_context(task_t* task) {
    /* Allocate stack for the task */
    task->context.stack_size = STACK_SIZE;
    task->context.stack_pointer = malloc(task->context.stack_size);
    
    if (!task->context.stack_pointer) {
        perror("Failed to allocate task stack");
        return false;
    }
    
    /* 
     * In a real implementation, we would set up the stack and registers
     * to prepare for context switching. This would be architecture-specific.
     * 
     * For educational purposes, we're using a simplified approach:
     * - We're not setting up a real context switch mechanism
     * - Instead, we'll use a cooperative approach with function calls
     */
    
    return true;
}

/**
 * Find the next task to run with priority-based fair scheduling
 * 
 * @return The next task to run, or NULL if no tasks are ready
 */
static task_t* scheduler_find_next_task(void) {
    if (!g_scheduler.ready_queue) {
        return NULL;
    }
    
    /* Start from the head of the ready queue */
    task_t* current = g_scheduler.ready_queue;
    task_t* best_task = NULL;
    priority_level_t highest_priority = PRIORITY_LOW;
    
    /* First pass: find highest priority READY task */
    while (current) {
        if (current->status == TASK_READY && current->priority > highest_priority) {
            highest_priority = current->priority;
            best_task = current;
        }
        /* For tasks of same priority, use fair scheduling criteria */
        else if (current->status == TASK_READY && current->priority == highest_priority) {
            /* If we don't have a task yet, use this one */
            if (!best_task) {
                best_task = current;
            }
            /* 1. Prefer tasks that haven't run yet (time_used == 0) */
            else if (current->time_used == 0 && best_task->time_used > 0) {
                best_task = current;
            }
            /* 2. Among tasks with the same run status, prefer tasks that have been waiting longer (earlier creation time) */
            else if ((current->time_used == 0 && best_task->time_used == 0) || 
                     (current->time_used > 0 && best_task->time_used > 0)) {
                /* Compare creation times - earlier creation time means task has been waiting longer */
                if (current->created_at.tv_sec < best_task->created_at.tv_sec || 
                    (current->created_at.tv_sec == best_task->created_at.tv_sec && 
                     current->created_at.tv_nsec < best_task->created_at.tv_nsec)) {
                    best_task = current;
                }
            }
        }
        current = current->next;
    }
    if (!best_task) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        
        current = g_scheduler.ready_queue;
        priority_level_t highest_sleep_priority = PRIORITY_LOW;
        task_t* best_sleep_task = NULL;
        
        /* Find all sleeping tasks that are ready to wake and pick highest priority */
        while (current) {
            if (current->status == TASK_SLEEPING) {
                /* Check if sleep time has elapsed */
                long elapsed_ms = (now.tv_sec - current->last_run.tv_sec) * 1000 + 
                                 (now.tv_nsec - current->last_run.tv_nsec) / 1000000;
                
                if (elapsed_ms >= current->time_slice) {
                    current->status = TASK_READY;
                    if (current->priority >= highest_sleep_priority) {
                        highest_sleep_priority = current->priority;
                        best_sleep_task = current;
                    }
                }
            }
            current = current->next;
        }
        
        if (best_sleep_task) {
            best_task = best_sleep_task;
            printf("Waking task %s from sleep\n", best_task->name);
        }
    }
    
    return best_task;
}

/**
 * Remove a task from the ready queue
 * 
 * @param task The task to remove
 * @return true if the task was removed, false if it wasn't in the queue
 */
static bool remove_task_from_queue(task_t* task) {
    if (!task || !g_scheduler.ready_queue) {
        return false;
    }
    
    /* Special case: task is at the head of the queue */
    if (g_scheduler.ready_queue == task) {
        g_scheduler.ready_queue = task->next;
        task->next = NULL;
        g_scheduler.task_count--;
        return true;
    }
    
    /* Search for the task in the queue */
    task_t* prev = g_scheduler.ready_queue;
    task_t* current = prev->next;
    
    while (current) {
        if (current == task) {
            /* Found the task, remove it */
            prev->next = current->next;
            task->next = NULL;
            g_scheduler.task_count--;
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    /* Task not found in queue */
    return false;
}

/**
 * Switch from the current task to the next task
 * 
 * @param next The task to switch to
 */
static void context_switch(task_t* next) {
    if (!next) {
        return;
    }
    
    /* Update current task if it exists */
    if (g_scheduler.current) {
        /* Record when this task last ran */
        clock_gettime(CLOCK_MONOTONIC, &g_scheduler.current->last_run);
        
        /* Handle task based on its status */
        if (g_scheduler.current->status == TASK_RUNNING) {
            g_scheduler.current->status = TASK_READY;
        } 
        else if (g_scheduler.current->status == TASK_FINISHED) {
            /* Task has finished, free its resources */
            task_t* finished_task = g_scheduler.current;
            
            /* Clear the current task pointer before potential deallocation */
            g_scheduler.current = NULL;
            
            /* Remove it from the queue */
            if (remove_task_from_queue(finished_task)) {
                printf("Removed finished task %s (ID: %u) from queue\n", 
                       finished_task->name, finished_task->id);
                
                /* Free task resources - but don't free the task itself yet */
                if (finished_task->context.stack_pointer && 
                    finished_task->context.stack_size > 0 && 
                    finished_task->context.stack_size <= STACK_SIZE) {
                    free(finished_task->context.stack_pointer);
                    finished_task->context.stack_pointer = NULL;
                    finished_task->context.stack_size = 0;
                }
                
                /* Now it's safe to free the task structure */
                free(finished_task);
            }
        }
    }
    
    /* Update and run the next task */
    g_scheduler.current = next;
    next->status = TASK_RUNNING;
    
    /* Reset time used */
    next->time_used = 0;
    
    printf("Switching to task: %s (ID: %u, Priority: %d)\n", 
           next->name, next->id, next->priority);
    
    /* 
     * In a real implementation, this is where we would perform an actual
     * context switch, saving and restoring CPU registers.
     * 
     * For educational purposes, we're using a simplified approach:
     * we'll simulate a context switch by calling the task function directly.
     */
    
    /* If the task is newly created, call its function for the first time */
    if (next->status == TASK_RUNNING && 
        next->time_used == 0 && 
        next->created_at.tv_sec == next->last_run.tv_sec &&
        next->created_at.tv_nsec == next->last_run.tv_nsec) {
        
        /* Use the task_trampoline function to properly handle task completion */
        task_trampoline(next);
        
        /* No need to manually mark as finished - trampoline handles this */
    } 
    /* For tasks that have yielded or woken from sleep, we don't call their function again */
    else if (next->status == TASK_READY) {
        /* Just update the status to RUNNING for tasks that were previously yielded */
        next->status = TASK_RUNNING;
        printf("Resuming task %s after yield/sleep\n", next->name);
    }
}

/**
 * Initialize the scheduler
 * 
 * This function must be called before any other scheduler functions.
 * 
 * @return true on success, false on failure
 */
bool scheduler_init(void) {
    /* Initialize the scheduler state */
    memset(&g_scheduler, 0, sizeof(g_scheduler));
    g_scheduler.default_time_slice = DEFAULT_TIME_SLICE;
    
    /* Set up timer for preemption */
    signal(SIGALRM, timer_handler);
    
    printf("Scheduler initialized\n");
    return true;
}

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
                   void* arg, const char* name) {
    if (!function) {
        fprintf(stderr, "Error: NULL function pointer provided to create_task\n");
        return NULL;
    }
    
    /* Allocate task structure */
    task_t* task = (task_t*)malloc(sizeof(task_t));
    if (!task) {
        perror("Failed to allocate task");
        return NULL;
    }
    
    /* Initialize the task */
    memset(task, 0, sizeof(task_t));
    task->id = next_task_id++;
    task->priority = priority;
    task->status = TASK_READY;
    task->function = function;
    task->arg = arg;
    task->time_slice = g_scheduler.default_time_slice;
    
    /* Set task name */
    if (name) {
        strncpy(task->name, name, sizeof(task->name) - 1);
        task->name[sizeof(task->name) - 1] = '\0';  /* Ensure null termination */
    } else {
        sprintf(task->name, "Task-%u", task->id);
    }
    
    /* Record creation time */
    clock_gettime(CLOCK_MONOTONIC, &task->created_at);
    task->last_run = task->created_at;
    
    /* Initialize the task's execution context */
    if (!init_task_context(task)) {
        free(task);
        return NULL;
    }
    
    printf("Created task: %s (ID: %u)\n", task->name, task->id);
    return task;
}

/**
 * Add a task to the scheduler
 * 
 * @param task The task to add
 * 
 * @return true on success, false on failure
 */
bool scheduler_add_task(task_t* task) {
    if (!task) {
        fprintf(stderr, "Error: NULL task provided to scheduler_add_task\n");
        return false;
    }
    
    /* Add task to the end of the ready queue */
    if (!g_scheduler.ready_queue) {
        g_scheduler.ready_queue = task;
    } else {
        task_t* current = g_scheduler.ready_queue;
        while (current->next) {
            current = current->next;
        }
        current->next = task;
    }
    
    /* Increment task count */
    g_scheduler.task_count++;
    
    printf("Added task to scheduler: %s (ID: %u)\n", task->name, task->id);
    return true;
}

/**
 * Setup the timer for preemptive scheduling
 */
static void setup_timer(void) {
    /* Configure the timer to expire after the time slice */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = g_scheduler.default_time_slice * 1000;
    timer.it_interval = timer.it_value;
    
    /* Start the timer */
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }
}

/**
 * Timer handler for preemptive scheduling
 */
static void timer_handler(int signum) {
    (void)signum;  /* Unused parameter */
    
    /* Force a context switch if we're still running */
    if (g_scheduler.running) {
        /* Instead of directly calling task_yield, set a flag to yield on next safe point
         * This avoids issues with calling non-async-signal-safe functions from a signal handler
         */
        /* Note: In a real implementation, we would use a flag and check it in scheduler_run
         * For this educational example, we'll still call task_yield() but with a note
         * about the proper approach
         */
        task_yield();
    }
}

/**
 * Start running the scheduler
 * 
 * This function will not return until scheduler_shutdown() is called
 * from within a task.
 * 
 * @return true if shutdown was clean, false on error
 */
bool scheduler_run(void) {
    if (g_scheduler.running) {
        fprintf(stderr, "Error: Scheduler is already running\n");
        return false;
    }
    
    if (g_scheduler.task_count == 0) {
        fprintf(stderr, "Error: No tasks to schedule\n");
        return false;
    }
    
    /* Mark scheduler as running */
    g_scheduler.running = true;
    
    printf("Starting scheduler with %u tasks\n", g_scheduler.task_count);
    
    /* Setup timer for preemptive scheduling */
    setup_timer();
    
    /* Run the scheduler loop */
    while (g_scheduler.running) {
        /* Check if current task is finished and needs cleanup */
        if (g_scheduler.current && g_scheduler.current->status == TASK_FINISHED) {
            task_t* finished_task = g_scheduler.current;
            g_scheduler.current = NULL;
            
            /* Remove from queue if still there */
            if (remove_task_from_queue(finished_task)) {
                printf("Cleaned up finished task %s (ID: %u)\n", 
                       finished_task->name, finished_task->id);
                
                /* Free task resources */
                if (finished_task->context.stack_pointer && 
                    finished_task->context.stack_size > 0 && 
                    finished_task->context.stack_size <= STACK_SIZE) {
                    free(finished_task->context.stack_pointer);
                    finished_task->context.stack_pointer = NULL;
                    finished_task->context.stack_size = 0;
                }
                
                /* Safety check before freeing */
                free(finished_task);
            }
            
            /* After cleaning up a finished task, find the next task immediately */
            continue;
        }
        
        /* Find the next task to run */
        task_t* next_task = scheduler_find_next_task();
        
        if (!next_task) {
            /* No tasks ready to run */
            if (g_scheduler.task_count == 0) {
                /* No more tasks, we're done */
                printf("No more tasks to run, shutting down scheduler\n");
                break;
            }
            
            /* Wait a bit and try again */
            usleep(1000);
            continue;
        }
        
        /* Switch to the next task */
        context_switch(next_task);
    }
    
    printf("Scheduler stopped\n");
    return true;
}

/**
 * Shutdown the scheduler
 * 
 * This will stop the scheduler and clean up resources.
 * Must be called from within a task.
 */
void scheduler_shutdown(void) {
    /* Stop the scheduler */
    g_scheduler.running = false;
    
    /* Disable the timer */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval = timer.it_value;
    setitimer(ITIMER_REAL, &timer, NULL);
    
    /* Save the current task pointer for special handling */
    task_t* current_task = g_scheduler.current;
    g_scheduler.current = NULL;  /* Clear global pointer early to prevent use-after-free */
    
    /* Check if current task is in the ready queue to prevent double free */
    bool current_task_in_queue = false;
    task_t* task_check = g_scheduler.ready_queue;
    while (task_check) {
        if (task_check == current_task) {
            current_task_in_queue = true;
            break;
        }
        task_check = task_check->next;
    }
    
    /* First, mark all tasks as finished to avoid dangling pointers */
    task_t* task_iter = g_scheduler.ready_queue;
    while (task_iter) {
        task_iter->status = TASK_FINISHED;
        task_iter = task_iter->next;
    }
    
    /* Take a copy of the ready queue and clear the global pointer */
    task_t* ready_queue = g_scheduler.ready_queue;
    g_scheduler.ready_queue = NULL;  /* Prevent access to freed tasks */
    
    /* Now free all tasks in the ready queue */
    task_t* current = ready_queue;
    task_t* next = NULL;
    
    while (current) {
        /* Save next pointer before modifying current */
        next = current->next;
        current->next = NULL;  /* Clear next pointer to prevent invalid access */
        
        /* Free task resources with validation */
        if (current->context.stack_pointer && 
            current->context.stack_size > 0 && 
            current->context.stack_size <= STACK_SIZE) {
            void* stack_to_free = current->context.stack_pointer;
            current->context.stack_pointer = NULL;
            current->context.stack_size = 0;
            free(stack_to_free);  /* Free stack after nullifying pointers */
        }
        
        /* Free the task structure */
        free(current);
        
        /* Move to next task */
        current = next;
    }
    
    /* Handle current task if it's not already freed through the queue */
    if (current_task && !current_task_in_queue) {
        /* Mark as finished */
        current_task->status = TASK_FINISHED;
        current_task->next = NULL;
        
        /* Free stack resources with validation */
        if (current_task->context.stack_pointer && 
            current_task->context.stack_size > 0 && 
            current_task->context.stack_size <= STACK_SIZE) {
            void* stack_to_free = current_task->context.stack_pointer;
            current_task->context.stack_pointer = NULL;
            current_task->context.stack_size = 0;
            free(stack_to_free);
        }
        
        /* Free the task structure */
        free(current_task);
    }
    
    /* Reset scheduler state after all tasks are freed */
    g_scheduler.current = NULL;
    g_scheduler.ready_queue = NULL;
    g_scheduler.running = false;
    g_scheduler.task_count = 0;
    g_scheduler.default_time_slice = DEFAULT_TIME_SLICE;
    
    printf("Scheduler shutdown complete\n");
}

/**
 * Yield execution from the current task
 * 
 * This allows the current task to voluntarily give up its time slice,
 * letting other tasks run.
 */
void task_yield(void) {
    /* This would trigger a context switch to the next task */
    if (!g_scheduler.running) {
        return;
    }
    
    /* In a real scheduler, we would save context and switch to next task */
    /* For educational purposes, we'll simulate this by returning from the task function */
    if (g_scheduler.current) {
        printf("Task %s yielding CPU\n", g_scheduler.current->name);
        
        /* If the task isn't finished, mark it as ready for the next run */
        if (g_scheduler.current->status == TASK_RUNNING) {
            g_scheduler.current->status = TASK_READY;
        }
        
        /* Record that this task has used some time */
        g_scheduler.current->time_used += 10; /* Arbitrary value */
        
        /* Force return to scheduler_run to pick the next task */
        /* The yield operation is simulated by returning from the task function */
        return;
    } else {
        printf("NULL task yielding CPU\n");
    }
}

/**
 * Sleep the current task for the specified number of milliseconds
 * 
 * @param ms Milliseconds to sleep
 */
void task_sleep_ms(uint32_t ms) {
    if (!g_scheduler.current) {
        return;
    }
    
    printf("Task %s sleeping for %u ms\n", g_scheduler.current->name, ms);
    
    /* Mark the task as sleeping */
    g_scheduler.current->status = TASK_SLEEPING;
    
    /* Record current time for wake-up calculation */
    clock_gettime(CLOCK_MONOTONIC, &g_scheduler.current->last_run);
    
    /* Set the time to wake up */
    g_scheduler.current->time_slice = ms;
    
    /* Yield the CPU */
    task_yield();
}
