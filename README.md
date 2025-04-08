```
   ______  __                               ____   _____
  / ____/ / /_   _____  ____    ____      / __ \ / ___/
 / /     / __ \ / ___/ / __ \  / __ \    / / / / \__ \ 
/ /___  / / / // /    / /_/ / / / / /   / /_/ / ___/ / 
\____/ /_/ /_//_/     \____/ /_/ /_/    \____/ /____/  
```

# ChronOS - A Minimalist Task Scheduler

ChronOS is an educational implementation of a priority-based task scheduler, designed to demonstrate the core concepts of operating system process scheduling in a simplified, understandable format.

## 🔍 Overview

ChronOS provides a lightweight, user-space implementation of task scheduling concepts typically found in operating system kernels. It serves as a learning tool for understanding how processes are managed, scheduled, and executed in modern operating systems.

Rather than attempting to implement a full-featured kernel scheduler, ChronOS focuses on clarity and educational value, making the fundamental concepts of scheduling accessible to students and enthusiasts interested in operating system design.

## ✨ Features

- **Priority-based scheduling**: Tasks with higher priority are executed first
- **Fair scheduling**: Tasks of equal priority get fair execution time
- **Cooperative multitasking**: Tasks can yield CPU time to other tasks
- **Task sleeping**: Tasks can sleep for specified time periods
- **Dynamic task creation**: Create and add tasks at runtime
- **Resource management**: Proper cleanup of task resources
- **Simple API**: Easy-to-understand interface for task manipulation

## 📦 Building ChronOS

### Prerequisites

- GCC or compatible C compiler
- Make
- Standard C library

### Build Commands

```bash
# Build the debug version with all logging enabled
make debug

# Build the release version with optimizations
make release

# Run the test suite
make run_test_debug

# Clean build artifacts
make clean
```

## 🚀 Usage Examples

### Basic Task Creation

```c
#include "minisched.h"

void my_task_function(void* arg) {
    printf("Task running with argument: %s\n", (char*)arg);
    // Task code here
}

int main() {
    // Initialize the scheduler
    scheduler_init();
    
    // Create a task with priority level 2
    task_t* task = create_task(my_task_function, PRIORITY_MEDIUM, 
                             "Hello ChronOS", "MyTask");
    
    // Add the task to the scheduler
    scheduler_add_task(task);
    
    // Run the scheduler
    scheduler_run();
    
    return 0;
}
```

### Task Yielding and Sleeping

```c
void cooperative_task(void* arg) {
    int i;
    for (i = 0; i < 5; i++) {
        printf("Task %s, iteration %d\n", (char*)arg, i);
        
        // Yield to allow other tasks to run
        task_yield();
    }
}

void sleeping_task(void* arg) {
    printf("Task %s going to sleep\n", (char*)arg);
    
    // Sleep for 500 milliseconds
    task_sleep_ms(500);
    
    printf("Task %s woke up\n", (char*)arg);
}
```

### Shutting Down the Scheduler

```c
void cleanup_task(void* arg) {
    // Do some work
    
    // Then shut down the scheduler
    scheduler_shutdown();
}
```

## 🛠️ Future Improvements

ChronOS currently implements fundamental scheduling concepts, but there are many exciting possibilities for future enhancements:

1. **True preemptive multitasking**: Implement true context switching with register saving/restoring
2. **Multi-core support**: Allow tasks to run on different CPU cores
3. **More scheduling algorithms**: Implement additional algorithms like CFS, EDF, etc.
4. **Inter-task communication**: Add message passing or shared memory primitives
5. **Task synchronization**: Implement mutexes, semaphores, and condition variables
6. **Real-time scheduling**: Add support for real-time constraints and deadlines
7. **Dynamic priority adjustment**: Allow task priorities to change based on behavior
8. **Task groups**: Implement task grouping and hierarchical scheduling
9. **Resource accounting**: Track CPU usage and other resources per task
10. **Plugin system**: Allow for custom scheduling policies to be plugged in

## 👥 Contributing

Contributions to ChronOS are welcome! Here are some ways you can contribute:

1. Report bugs and suggest features by opening issues
2. Submit pull requests with bug fixes or enhancements
3. Improve documentation and write tutorials
4. Share your experience using ChronOS in educational settings

### Contribution Guidelines

- Follow the existing code style and naming conventions
- Include appropriate comments for new code
- Add or update tests for new functionality
- Keep the focus on educational value and code clarity
- Document your changes thoroughly

## 📄 License

ChronOS is licensed under the MIT License. See the LICENSE file for more details.

## 👤 Author

Created by Alex (A5873)  
Email: ngugialex540@gmail.com

---

ChronOS is designed for educational purposes and is not intended for production use. It provides a simplified implementation of scheduling concepts to facilitate learning and experimentation.

# MiniSched: A Minimalist Process Scheduler

## Project Overview

MiniSched is an educational implementation of a simple process scheduler designed to demonstrate fundamental operating system concepts. It provides a clear, concise codebase that illustrates how task scheduling works in modern operating systems, without the complexity found in production schedulers.

### Educational Purpose

This project serves several educational purposes:

- **Demonstrate Scheduling Concepts**: Illustrates fundamental scheduling algorithms and techniques
- **Show Task Management**: Provides a practical example of task creation, execution, and cleanup
- **Explain Context Switching**: Demonstrates simplified task context switching
- **Illustrate Cooperative Multitasking**: Shows how tasks can voluntarily yield control
- **Present Resource Management**: Demonstrates memory allocation and cleanup for tasks

## Architecture and Design

MiniSched implements a simplified round-robin scheduler with the following core components:

### Core Components

1. **Task Structure**: Represents a schedulable unit of work with metadata
2. **Ready Queue**: Maintains a linked list of tasks ready for execution
3. **Scheduler Loop**: Main execution engine that selects and runs tasks
4. **Context Switching**: Simplified mechanism to switch between tasks
5. **Task Synchronization**: Methods for tasks to yield control or sleep

### Design Decisions

- **Simplified Context**: Uses function calls rather than actual register/stack manipulation
- **Cooperative Scheduling**: Primarily relies on tasks to yield control
- **Round-Robin Algorithm**: Each task gets a time slice before moving to the next
- **Task States**: Implements READY, RUNNING, SLEEPING, BLOCKED, and FINISHED states
- **Memory Management**: Handles stack allocation and cleanup for tasks

### State Diagram

```
                   ┌─────────┐
                   │  READY  │◄────────────────┐
                   └────┬────┘                 │
                        │                      │
                        │ Scheduled            │
                        ▼                      │
┌──────────┐       ┌─────────┐        ┌────────────┐
│ FINISHED │◄──────┤ RUNNING ├───────►│  SLEEPING  │
└──────────┘       └────┬────┘        └────────────┘
                        │                    ▲
                        │ Resource           │
                        │ unavailable        │
                        ▼                    │
                   ┌─────────┐              │
                   │ BLOCKED │──────────────┘
                   └─────────┘  Resource
                                available
```

## Implementation Details

### Task Lifecycle

1. **Creation**: Tasks are created with `create_task()`, allocating memory and initializing state
2. **Scheduling**: Tasks are added to the ready queue with `scheduler_add_task()`
3. **Execution**: The scheduler selects tasks to run based on their state and priority
4. **Yielding**: Tasks can yield control with `task_yield()` or sleep with `task_sleep_ms()`
5. **Completion**: Tasks mark themselves as FINISHED when their function returns
6. **Cleanup**: The scheduler removes finished tasks and frees their resources

### Key Features

#### Task Trampoline

The task trampoline function serves as a wrapper around task functions, handling their completion:

```c
static void task_trampoline(struct task* task) {
    /* Execute the task function */
    task->function(task->arg);
    
    /* Task has completed, mark it as finished */
    task->status = TASK_FINISHED;
    
    /* Return to let scheduler handle cleanup */
}
```

#### Context Switching

A simplified context switch that selects the next task to run:

```c
static void context_switch(task_t* next) {
    /* Update current task if it exists */
    if (g_scheduler.current) {
        /* Handle current task based on its status */
        ...
    }
    
    /* Update and run the next task */
    g_scheduler.current = next;
    next->status = TASK_RUNNING;
    
    /* Execute the task */
    if (next->status == TASK_RUNNING) {
        task_trampoline(next);
    }
}
```

#### Task Removal

When tasks finish, they're removed from the queue and their resources are freed:

```c
static bool remove_task_from_queue(task_t* task) {
    /* Remove task from linked list */
    ...
    
    /* Free resources */
    if (task->context.stack_pointer) {
        free(task->context.stack_pointer);
    }
    
    /* Free the task itself */
    free(task);
    
    return true;
}
```

## Build and Usage Instructions

### Prerequisites

- GCC compiler or compatible alternative
- Standard C libraries
- Make build system

### Building

The project includes a Makefile with several build targets:

```bash
# Build release version (optimized)
make release

# Build debug version (with debug symbols)
make debug

# Build and run debug version
make run_debug

# Build and run release version
make run

# Clean build artifacts
make clean
```

### Running

After building, run the example program:

```bash
# Run the debug build
./minisched_debug

# Run the release build
./minisched
```

## Example Program

The included `main.c` demonstrates various task types and scheduling behaviors:

### Task Types

1. **Compute Task**: CPU-intensive task that computes prime numbers
2. **Periodic Task**: Wakes up at regular intervals, performs work, then sleeps
3. **Cooperative Task**: Yields frequently to demonstrate cooperation
4. **Cleanup Task**: Runs after a delay and shuts down the scheduler

### Example Output

```
MiniSched Example - Demonstrating task scheduling
================================================

Scheduler initialized
Created task: ComputeTask (ID: 1)
Created task: PeriodicTask (ID: 2)
Created task: CoopTask (ID: 3)
Added task to scheduler: ComputeTask (ID: 1)
Added task to scheduler: PeriodicTask (ID: 2)
Added task to scheduler: CoopTask (ID: 3)
Created task: CleanupTask (ID: 4)
Added task to scheduler: CleanupTask (ID: 4)
All tasks created and added to scheduler
Starting scheduler - it will run for approximately 5 seconds

Starting scheduler with 4 tasks
Switching to task: ComputeTask (ID: 1)
[ComputeTask] Starting compute task
[ComputeTask] Computing primes (iteration 1/5)...
...
[ComputeTask] Compute task completed
Task ComputeTask (ID: 1) has finished
Cleaned up finished task ComputeTask (ID: 1)
...
[Cleanup] Shutting down scheduler
Scheduler shutdown complete
[Cleanup] Cleanup task completed
Task CleanupTask (ID: 4) has finished
Scheduler stopped

Scheduler has shut down
Example completed successfully
```

## Code Structure

### Key Files

- **minisched.h**: Public API declarations and data structures
- **minisched.c**: Implementation of the scheduler
- **main.c**: Example tasks and demonstration code
- **Makefile**: Build system configuration

### Important Functions

| Function | Description |
|----------|-------------|
| `scheduler_init()` | Initializes the scheduler |
| `create_task()` | Creates a new task with function and priority |
| `scheduler_add_task()` | Adds a task to the scheduler queue |
| `scheduler_run()` | Starts the scheduler main loop |
| `task_yield()` | Voluntarily yields CPU from current task |
| `task_sleep_ms()` | Makes a task sleep for specified milliseconds |
| `scheduler_shutdown()` | Stops the scheduler and cleans up |

## Future Enhancements

MiniSched could be extended with additional features to further demonstrate OS concepts:

1. **Preemptive Scheduling**: Implement true preemption using signals/timers
2. **Priority-Based Scheduling**: Enhance the scheduler to respect priority levels
3. **Resource Management**: Add semaphores and mutex implementations
4. **Inter-Task Communication**: Implement message passing between tasks
5. **Task Dependencies**: Allow specifying task dependencies and ordering
6. **Real Context Switching**: Implement actual register/stack manipulation
7. **Dynamic Priority Adjustment**: Implement priority aging and boosting
8. **I/O Scheduling**: Add simulated I/O operations and scheduling

## Learning Opportunities

MiniSched provides hands-on learning for several important OS concepts:

- Process and thread management
- CPU scheduling algorithms
- Resource allocation and deallocation
- Race conditions and synchronization
- Memory management
- Task state transitions
- Cooperative vs. preemptive multitasking

## License

This project is intended for educational purposes. Feel free to use, modify, and distribute the code for learning and teaching.

## Contribution

Contributions that enhance the educational value of this project are welcome. Please maintain the clarity and simplicity of the code while adding new features or improvements.

# MiniSched: A Minimalist Process Scheduler

## Overview
MiniSched is a lightweight, educational implementation of a process scheduler written in C. It demonstrates the core principles of process scheduling in operating systems using a simple round-robin algorithm.

## Goals and Features
- Implement a basic round-robin scheduling algorithm
- Demonstrate process creation, management, and context switching
- Provide a simple API for task submission and management
- Educational tool for understanding OS scheduler fundamentals
- Minimal dependencies (POSIX-compliant system only)

## Building the Project
```bash
# Clone the repository
git clone https://github.com/yourusername/mini-scheduler.git
cd mini-scheduler

# Build the project
make

# Run tests
make test
```

## Usage
```c
#include "minisched.h"

int main() {
    // Initialize the scheduler
    scheduler_init();
    
    // Create and add tasks
    task_t task1 = create_task(my_function, priority_normal);
    scheduler_add_task(task1);
    
    // Start the scheduler
    scheduler_run();
    
    // Cleanup
    scheduler_shutdown();
    return 0;
}
```

## Educational Purpose
This project is designed for educational purposes to help understand the fundamentals of process scheduling. It is not intended for production use and lacks many features of a full-featured scheduler.

## License
MIT License

