#pragma once

#include <stdint.h>

// type definitions
typedef uint64_t paddr_t;
typedef uint64_t vaddr_t;

typedef uint64_t pte_t;
typedef uint64_t* pagetable_t;

typedef int pid_t;
typedef int64_t ssize_t;    // ssize_t is an signed counterpart of size_t
typedef int64_t time_t;