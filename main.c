/**
 * main.c - Example usage of the Minimalist Process Scheduler
 * 
 * This example demonstrates different types of tasks and how they
 * interact with the scheduler.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "minisched.h"

/* Global flag to signal tasks to exit */
static volatile bool should_exit = false;

/**
 * Task 1: Compute-intensive task
 * 
 * This task simulates a CPU-bound workload by calculating
 * prime numbers. It demonstrates how a long-running computation
 * affects scheduling.
 */
void compute_task(void* arg) {
    const char* name = (const char*)arg;
    int count = 0;
    int num = 2;
    int iterations = 0;
    
    printf("[%s] Starting compute task\n", name);
    
    while (!should_exit && iterations < 5) {
        iterations++;
        printf("[%s] Computing primes (iteration %d/5)...\n", name, iterations);
        
        /* Find the next 10 prime numbers */
        count = 0;
        while (count < 10 && !should_exit) {
            bool is_prime = true;
            
            /* Check if num is prime */
            for (int i = 2; i * i <= num; i++) {
                if (num % i == 0) {
                    is_prime = false;
                    break;
                }
            }
            
            if (is_prime) {
                printf("[%s] Found prime: %d\n", name, num);
                count++;
            }
            
            num++;
            
            /* Occasionally yield to demonstrate cooperation */
            if (num % 50 == 0) {
                printf("[%s] Yielding during computation\n", name);
                task_yield();
            }
        }
        
        /* Yield after finding 10 primes */
        printf("[%s] Completed iteration, yielding...\n", name);
        task_yield();
    }
    
    printf("[%s] Compute task completed\n", name);
}

/**
 * Task 2: Periodic task
 * 
 * This task wakes up periodically, performs a small amount of work,
 * then goes back to sleep. It demonstrates how the scheduler handles
 * tasks that sleep and wake up.
 */
void periodic_task(void* arg) {
    const char* name = (const char*)arg;
    int iterations = 0;
    
    printf("[%s] Starting periodic task\n", name);
    
    while (!should_exit && iterations < 10) {
        iterations++;
        printf("[%s] Waking up (iteration %d/10)\n", name, iterations);
        
        /* Simulate some light work */
        int result = 0;
        for (int i = 0; i < 1000; i++) {
            result += i;
        }
        
        printf("[%s] Work completed, result = %d\n", name, result);
        printf("[%s] Going to sleep for 100ms...\n", name);
        
        /* Sleep for a while */
        task_sleep_ms(100);
    }
    
    printf("[%s] Periodic task completed\n", name);
}

/**
 * Task 3: Cooperative yielding task
 * 
 * This task yields very frequently, demonstrating cooperative multitasking.
 * It shows how a task can voluntarily give up CPU time.
 */
void cooperative_task(void* arg) {
    const char* name = (const char*)arg;
    int iterations = 0;
    
    printf("[%s] Starting cooperative task\n", name);
    
    while (!should_exit && iterations < 20) {
        iterations++;
        
        printf("[%s] Working (iteration %d/20)\n", name, iterations);
        
        /* Do a very small amount of work */
        int sum = 0;
        for (int i = 0; i < 100; i++) {
            sum += i;
        }
        
        printf("[%s] Sum = %d, yielding immediately\n", name, sum);
        
        /* Yield after every small unit of work */
        task_yield();
    }
    
    printf("[%s] Cooperative task completed\n", name);
}

/**
 * Cleanup task
 * 
 * This task runs after all other tasks have had a chance to run.
 * It sets the exit flag and shuts down the scheduler.
 */
void cleanup_task(void* arg) {
    const int delay_seconds = *((int*)arg);
    
    printf("[Cleanup] Starting cleanup task\n");
    printf("[Cleanup] Will shut down scheduler after %d seconds\n", delay_seconds);
    
    /* Wait to let other tasks run */
    for (int i = 0; i < delay_seconds; i++) {
        printf("[Cleanup] Shutdown in %d seconds...\n", delay_seconds - i);
        task_sleep_ms(1000);
    }
    
    /* Signal all tasks to exit */
    printf("[Cleanup] Setting exit flag\n");
    should_exit = true;
    
    /* Give other tasks a chance to see the exit flag */
    task_yield();
    task_sleep_ms(500);
    
    /* Shutdown the scheduler */
    printf("[Cleanup] Shutting down scheduler\n");
    scheduler_shutdown();
    
    printf("[Cleanup] Cleanup task completed\n");
}

int main(int argc, char* argv[]) {
    /* Suppress unused parameter warnings */
    (void)argc;
    (void)argv;
    printf("MiniSched Example - Demonstrating task scheduling\n");
    printf("================================================\n\n");
    
    /* Initialize the scheduler */
    if (!scheduler_init()) {
        fprintf(stderr, "Failed to initialize scheduler\n");
        return EXIT_FAILURE;
    }
    
    /* Create tasks */
    task_t* task1 = create_task(compute_task, PRIORITY_NORMAL, "ComputeTask", "ComputeTask");
    task_t* task2 = create_task(periodic_task, PRIORITY_HIGH, "PeriodicTask", "PeriodicTask");
    task_t* task3 = create_task(cooperative_task, PRIORITY_LOW, "CoopTask", "CoopTask");
    
    /* Check if task creation was successful */
    if (!task1 || !task2 || !task3) {
        fprintf(stderr, "Failed to create one or more tasks\n");
        return EXIT_FAILURE;
    }
    
    /* Add tasks to the scheduler */
    scheduler_add_task(task1);
    scheduler_add_task(task2);
    scheduler_add_task(task3);
    
    /* Create and add the cleanup task that will eventually shut down the scheduler */
    int cleanup_delay = 5; /* seconds before shutdown */
    task_t* cleanup = create_task(cleanup_task, PRIORITY_REALTIME, &cleanup_delay, "CleanupTask");
    
    if (!cleanup) {
        fprintf(stderr, "Failed to create cleanup task\n");
        return EXIT_FAILURE;
    }
    
    scheduler_add_task(cleanup);
    
    printf("All tasks created and added to scheduler\n");
    printf("Starting scheduler - it will run for approximately %d seconds\n\n", cleanup_delay);
    
    /* Run the scheduler - this will block until scheduler_shutdown() is called */
    if (!scheduler_run()) {
        fprintf(stderr, "Scheduler encountered an error\n");
        return EXIT_FAILURE;
    }
    
    printf("\nScheduler has shut down\n");
    printf("Example completed successfully\n");
    
    return EXIT_SUCCESS;
}

