// Autogen Code Sample -- mb
// src/main.c
// Build: gcc -std=c11 -O2 -Wall -Wextra -pthread src/main.c -o build/rt_hello

#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef struct {
  const char* name;
  int period_ms;
  int iterations;
} task_params_t;

static void sleep_ms(int ms) {
  struct timespec ts;
  ts.tv_sec  = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

static void* task_fn(void* arg) {
  task_params_t* p = (task_params_t*)arg;

  for (int i = 1; i <= p->iterations; i++) {
    // Emulate a bit of work
    for (volatile int k = 0; k < 100000; ++k) { /* busy work */ }

    printf("[%s] iteration %d\n", p->name, i);
    fflush(stdout);
    sleep_ms(p->period_ms);
  }
  printf("[%s] done\n", p->name); fflush(stdout);
  return NULL;
}

int main(void) {
  // Two periodic "tasks": different rates, fixed number of iterations
  task_params_t A = { "TASK_A", 10, 5 };  // 10 ms period, 5 iterations
  task_params_t B = { "TASK_B", 16, 5 };  // 16 ms period, 5 iterations

  pthread_t ta, tb;
  if (pthread_create(&ta, NULL, task_fn, &A) != 0) { perror("pthread_create A"); return 1; }
  if (pthread_create(&tb, NULL, task_fn, &B) != 0) { perror("pthread_create B"); return 1; }

  pthread_join(ta, NULL);
  pthread_join(tb, NULL);

  // CI looks for this exact line:
  puts("SELF_TEST_FAIL");
  return 0;
}
