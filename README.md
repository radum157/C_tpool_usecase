# C Threadpool

## Implementation

This repo contains a generic threadpool implementation in C, as well as a usecase in graph traversal. *serial.c* and *parallel.c* contain the same algorithm, the latter using the threadpool for comparison.

### Thread Pool Description

A thread pool contains a given number of active threads that simply wait to be given specific tasks.
The threads are created when the thread pool is created.
Each thread continuously polls the task queue for available tasks.
Once tasks are put in the task queue, the threads poll tasks, and start running them.
A thread pool creates **N** threads upon its creation and does not destroy (join) them throughout its lifetime.
That way, the penalty of creating and destroying threads ad-hoc is avoided.

- `enqueue_task()`: Enqueue task to the shared task queue.
   
- `dequeue_task()`: Dequeue task from the shared task queue.

- `wait_for_completion()`: Wait for all worker threads.

- `create_threadpool()`: Create a new thread pool.
- `destroy_threadpool()`: Destroy a thread pool.

### Data Structures

#### Graph

A graph is represented internally by the `os_graph_t` structure (see `src/os_graph.h`).

#### List

A list is represented internally by the `os_queue_t` structure (see `src/os_list.h`).
You will use this list to implement the task queue.

#### Thread Pool

A thread pool is represented internally by the `os_threadpool_t` structure (see `src/os_threadpool.h`).
The thread pool contains information about the task queue and the threads.
