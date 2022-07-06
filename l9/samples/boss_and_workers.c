#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Program that demonstrates the usage of conditional variables (initialization, signaling and broadcasting condition
 * changes, as well as destroying such variables).
 * Scenario:
 *  1. There are N workers (threads) who must perform a number of tasks. That number is called the workload, and each worker has the same workload.
 *  2. There is a (lazy) boss thread which monitors the progress of the workers. Whenever a worker finishes a task, it must notify the boss.
 *  3. Whenever a worker finishes all its tasks, it goes to a barrier and notifies the boss about the fact that it is waiting at the barrier.
 *  4. When all the workers finished their tasks and are at the barrier, the boss will raise the barrier.
 * Compile the program using the "gcc -Wall -Werror -DDEBUG -DWORKER_COUNT=<number-of-workers> -DWORKLOAD=<number-of-tasks-per-worker> -pthread cond_vars.c -o cond_vars"
 */

pid_t gettid();

// Logging
#ifdef DEBUG
#define log(msg) printf("[PID: %d][TID: %d][%s][%s][%d]%s", getpid(), gettid(), __FILE__, __FUNCTION__, __LINE__, msg)
#define logf(fmt, ...) {char format[1024]; snprintf(format, sizeof(format), "[PID: %d][TID: %d][%s][%s][%d]%s", getpid(), gettid(), __FILE__, __FUNCTION__, __LINE__, fmt); printf(format, __VA_ARGS__); }
#define error(msg) fprintf(stderr, "[PID: %d][TID: %d][%s][%s][%d]%s: %s\n", getpid(), gettid(), __FILE__, __FUNCTION__, __LINE__, msg, strerror(errno))
#define errorf(fmt, ...) {char format[1024]; snprintf(format, sizeof(format), "[PID: %d][TID: %d][%s][%s][%d]%s: %s\n", getpid(), gettid(), __FILE__, __FUNCTION__, __LINE__, fmt, strerror(errno)); printf(format, __VA_ARGS__); }
#else
#define log(msg)
#define logf(fmt, ...) 
#define error(msg) 
#define errorf(fmt, ...) 
#endif

// Condition checks used when using system calls whose return values have to be validated
#define check(condition) if (condition) { break; }
#define check_log(condition, message) if (condition) { log(message); break; }
#define check_logf(condition, fmt, ...) if (condition) { logf(fmt, __VA_ARGS__); break; } 
#define check_error(condition, message) if (condition) { error(message); break; }
#define check_errorf(condition, fmt, ...) if (condition) { errorf(fmt, __VA_ARGS__); break; }

/**
 * @brief Function for decrementing a semaphore
 * @param semid The identifier of the semaphore set
 * @param semnum The index of the semaphore in the semaphore set
 * @param value The value by which the semaphore is to be decremented
 */
void P(int semid, int semnum, int value)
{
    struct sembuf op = {semnum, -value, 0};
    semop(semid, &op, 1);
}

/**
 * @brief Function for incrementing a semaphore
 * @param semid The identifier of the semaphore set
 * @param semnum The index of the semaphore in the semaphore set
 * @param value The value by which the semaphore is to be incremented
 */
void V(int semid, int semnum, int value)
{
    struct sembuf op = {semnum, +value, 0};
    semop(semid, &op, 1);
}

/**
 * @brief Data structure which is to be passed to the threads
 * in order to avoid using global variables.
 */
typedef struct 
{
    unsigned int counter;
    bool barrier_raised;
    bool counter_mutex_initialized;
    bool counter_cond_initialized;
    bool barrier_mutex_initialized;
    bool barrier_cond_initialized;
    int semid;
    pthread_mutex_t counter_mutex;
    pthread_cond_t counter_condvar;
    pthread_mutex_t barrier_mutex;
    pthread_cond_t barrier_condvar;
} context_t;

/**
 * @brief Function which simulates a worker in a factory. Each worker has the same workload that has to accomplish.
 * @param context The execution context of the thread
 * @return void* Value queried by the parent thread.
 */
void* worker(void* context)
{
    context_t* ctx;

    ctx = (context_t*)context;

    for (int i = 0; i < WORKLOAD; i++)
    {
        // Critical section
        pthread_mutex_lock(&ctx->counter_mutex);
        log("Performing my next task ...\n");
        ctx->counter++;
        // Small sleep intervals in order to give the scheduler time
        // to resume the boss' execution.
        usleep(1000);
        log("Task done! Signaling the change ...\n");
        // Notify the "boss" thread about the fact that another task has been completed
        pthread_cond_signal(&ctx->counter_condvar);
        pthread_mutex_unlock(&ctx->counter_mutex);
    }

    // Wait for the boss thread to raise the barrier
    pthread_mutex_lock(&ctx->barrier_mutex);
    while(!ctx->barrier_raised)
    {
        log("Barrier not raised :(! Signal presence and wait ...\n");
        // Use semaphores just to make 100% sure that the boss thread does
        // not call the broadcast function before all the workers arrived to the barrier.
        V(ctx->semid, 0, 1);
        // Just wait and hope for the best from the boss :)
        pthread_cond_wait(&ctx->barrier_condvar, &ctx->barrier_mutex);
    }
    log("Barrier raised! I am freeeee :)\n");
    pthread_mutex_unlock(&ctx->barrier_mutex);

    return NULL;
}

void* boss(void* context)
{
    context_t* ctx;

    ctx = (context_t*)context;

    // The "boss" thread wants to check what the "worker" threads did so far
    pthread_mutex_lock(&ctx->counter_mutex);
    log("Waiting for my workers to increment the counter ...\n");
    // The boss expects a given work quantity to be done by the workers
    while (ctx->counter != WORKER_COUNT * WORKLOAD)
    {
        // If the work was not done, the boss goes back to sleep.
        // Yeah, he is a very lazy boss who likes to torture workers.
        log("These workers did not increment the counter for me. Hmmmm. I will go back to sleep.\n");
        pthread_cond_wait(&ctx->counter_condvar, &ctx->counter_mutex);
        // The boss was just notified that another work item has been submitted.
        // Let's hope that this time the boss will be satisfied with the results :|
        log("I am awake. Let me check if the workers did increment the counter for me\n");
    }
    // Wheeew, finally
    log("Finally! Ok, I'll let them get to the barrier and I will raise the barrier.\n");
    pthread_mutex_unlock(&ctx->counter_mutex);

    // Again, use the semaphore in order to make 100% sure that ALL the workers are at the barrier
    P(ctx->semid, 0, WORKER_COUNT);
    pthread_mutex_lock(&ctx->barrier_mutex);
    ctx->barrier_raised = true;
    log("I just raised the barrier. Now I will notify all the workers that they are free.\n");
    // Use broadcast to signal the workers that they are free.
    pthread_cond_broadcast(&ctx->barrier_condvar);
    pthread_mutex_unlock(&ctx->barrier_mutex);

    return NULL;
}

int main()
{
    pthread_t threads[WORKER_COUNT + 1];
    context_t context;

    // Initialize the data structures
    memset(&context, 0, sizeof(context_t));
    memset(threads, 0, sizeof(threads));

    // Block of code where we can initialize our resources in a consistent manner and interrupt the execution whenever
    // an error occurs. It is very similar with a "try" block from Java
    do
    {
        check_error(pthread_mutex_init(&context.counter_mutex, NULL) != 0, "Failed to initialize the mutex for the counter");
        context.counter_mutex_initialized = true;
        check_error(pthread_mutex_init(&context.barrier_mutex, NULL) != 0, "Failed to initialize the mutex for the barrier");
        context.barrier_mutex_initialized = true;
        check_error(pthread_cond_init(&context.counter_condvar, NULL) != 0, "Failed to initialize the condition variable for the counter");
        context.counter_cond_initialized = true;
        check_error(pthread_cond_init(&context.barrier_condvar, NULL) != 0, "Failed to initialize the condition variable for the barrier");
        context.barrier_cond_initialized = true;
        check_error((context.semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) <= 0, "Failed to create semaphore set");
        check_errorf(semctl(context.semid, 0, SETVAL, 0) != 0, "Failed to initialize semaphore %d", context.semid);

        for (int i = 0; i < WORKER_COUNT; i++)
        {
            check_errorf(pthread_create(&threads[i], NULL, worker, &context) != 0, "Failed to create worker thread with index %d", i);
        }

        check_error(pthread_create(&threads[WORKER_COUNT], NULL, boss, &context) != 0, "Failed to create the boss thread");
    } while (0);

    // This is equivalent with the "finally" block from Java
    // wait for the created threads to terminate their execution
    for (int i = 0; i < WORKER_COUNT + 1; i++)
    {
        if(threads[i] != 0)
        {
            pthread_join(threads[i], NULL);
        }
    }

    // If the counter's mutex was initialized, destroy it
    if (context.counter_mutex_initialized)
    {
        pthread_mutex_destroy(&context.counter_mutex);
    }

    // If the counter's conditional variable was initialized, destroy it
    if (context.counter_cond_initialized)
    {
        pthread_cond_destroy(&context.counter_condvar);
    }

    // If the barrier's mutex was initialized, destroy it
    if (context.barrier_mutex_initialized)
    {
        pthread_mutex_destroy(&context.barrier_mutex);
    }

    // If the barrier's conditional variable was initialized, destroy it
    if (context.barrier_cond_initialized)
    {
        pthread_cond_destroy(&context.barrier_condvar);
    }

    // Remove the semaphore set if it was initialized
    if (context.semid > 0)
    {
        semctl(context.semid, 0, IPC_RMID, 0);
    }

    exit(0);
}