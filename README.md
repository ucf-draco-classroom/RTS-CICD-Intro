# CI/CD Primer: Two-Task C Program

This mini project is a **zero-hardware** introduction to CI/CD for real-time systems.  
You’ll build a small C program that emulates **two periodic “tasks”** using POSIX threads, run it automatically in **GitHub Actions**, and publish a binary on a **tagged release**.

> **What you’ll learn**
> - **CI** (Continuous Integration): Compile, run, and automatically **assert** program health in the cloud.
> - **CD** (Continuous Delivery): Package and attach build artifacts to a GitHub **Release** when you tag a version.
> - **Real-time connection:** Two periodic activities (“tasks”) that signal success with a `SELF_TEST_PASS` banner.

---

## Repository layout

```
.
├─ src/
│  └─ main.c               # two "tasks" using pthreads
├─ Makefile                # build + run + assert
└─ .github/
   └─ workflows/
      ├─ ci.yaml            # CI pipeline (build + run + check)
      └─ release.yaml       # CD pipeline (on tag -> release binary)
```

---

## Quick start

1. **Fork** this repository.  
2. **Commit & push** — CI runs automatically on every push and pull request.  
3. **(Optional) Tag a release** to trigger CD:
   ```bash
   git tag v0.1.0
   git push origin v0.1.0
   ```
   A new GitHub **Release** will be created with the compiled binary attached.

---

## How the CI works

- The workflow in `.github/workflows/ci.yml` runs on **Ubuntu**.
- Steps:
  1. Install build tools (`build-essential`).
  2. `make` to build `rt_hello`.
  3. `make test` to run it and capture output.
  4. **Assert pass**: CI greps for the exact line `SELF_TEST_PASS`. If missing, the job fails.
  5. Uploads `build/output.txt` as a job **artifact** so you can inspect the run.

You can view logs and artifacts under **Actions → C two-tasks CI → (latest run)**.

---

## How the CD (release) works

- The workflow in `.github/workflows/release.yml` runs **only** when you push a **tag** like `v1.2.3`.
- It builds the program and then creates a **GitHub Release** with the compiled `rt_hello` binary attached.
- Find it under **Releases** in your repo after the workflow completes.

---

## Local build (optional, for your laptop)

You can do everything in the browser with GitHub Actions, but local runs are great for fast iteration.

### macOS
```bash
xcode-select --install
# If you use Homebrew:
# brew install gcc make
make
make test
```

### Linux (Debian/Ubuntu)
```bash
sudo apt-get update && sudo apt-get install -y build-essential
make
make test
```

Expected output (line ordering may differ):
```
[TASK_A] iteration 1
[TASK_B] iteration 1
...
[TASK_A] done
[TASK_B] done
SELF_TEST_PASS
```

---

## The code you’re building

### `src/main.c`
```c
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
    for (volatile int k = 0; k < 100000; ++k) { /* emulate work */ }
    printf("[%s] iteration %d\n", p->name, i);
    fflush(stdout);
    sleep_ms(p->period_ms);
  }
  printf("[%s] done\n", p->name); fflush(stdout);
  return NULL;
}

int main(void) {
  task_params_t A = { "TASK_A", 10, 5 };
  task_params_t B = { "TASK_B", 16, 5 };
  pthread_t ta, tb;
  pthread_create(&ta, NULL, task_fn, &A);
  pthread_create(&tb, NULL, task_fn, &B);
  pthread_join(ta, NULL);
  pthread_join(tb, NULL);
  puts("SELF_TEST_PASS");
  return 0;
}
```

### `Makefile`
```make
CC       := gcc
CFLAGS   := -std=c11 -O2 -Wall -Wextra -pthread
LDFLAGS  := -pthread
BUILD    := build
TARGET   := $(BUILD)/rt_hello
SRC      := src/main.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(TARGET)
	./$(TARGET) | tee $(BUILD)/output.txt
	@grep -q "SELF_TEST_PASS" $(BUILD)/output.txt

clean:
	rm -rf $(BUILD)
```

### `.github/workflows/ci.yaml`
```yaml
name: C two-tasks CI

on:
  push:
  pull_request:

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install build tools
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential
      - name: Build
        run: make -j
      - name: Run & assert
        run: make test
      - name: Upload run logs (artifact)
        uses: actions/upload-artifact@v4
        with:
          name: rt_hello-output
          path: build/output.txt
```

### `.github/workflows/release.yaml`
```yaml
name: Release binary

on:
  push:
    tags:
      - 'v*.*.*'

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install build tools
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential
      - name: Build
        run: make -j
      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: rt_hello
          path: build/rt_hello

  release:
    needs: build
    runs-on: ubuntu-latest
    permissions:
      contents: write
    steps:
      - uses: actions/download-artifact@v4
        with:
          name: rt_hello
          path: dist
      - name: Create GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          files: dist/rt_hello
```

---

## Suggested experiments

| Goal | Edit | CI Effect |
|------|------|-----------|
| Test CI failure | Comment out `SELF_TEST_PASS` | CI fails |
| Verify iteration count | Add `grep -c` checks in `Makefile` | CI counts lines |
| Deadline demo | Add `DEADLINE_MISS` when timing exceeds limit | CI fails if found |
| Style gate | Add `clang-format -n` | CI enforces formatting |
| Portability | Add matrix for `macos-latest` | CI runs on both OSes |


---

## Checklist

- [ ] CI runs successfully (`SELF_TEST_PASS` in output).  
- [ ] Artifact `rt_hello-output` uploaded.  
- [ ] Tag a release (`v0.1.0`) → binary attached.  
- [ ] Implement one extension (iteration counting, deadline check, style gate, or matrix build).

---

