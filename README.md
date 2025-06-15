# Custom Malloc – Memory Allocator in C

![C](https://img.shields.io/badge/C-Language-blue)
![Memory Management](https://img.shields.io/badge/Memory-Allocator-purple)
![Low-Level](https://img.shields.io/badge/Systems-Programming-orange)

This project is a minimalist implementation of a **custom memory allocator** in C, replicating the behaviour of standard functions like `malloc()`, `free()`, and `realloc()`. It supports **three memory allocation strategies**:
- **First Fit**
- **Best Fit**
- **Worst Fit**

All strategies use an **embedded free list** for efficient allocation and deallocation.

---

## Features

- `mm_malloc(size_t size)` – Allocate memory
- `mm_realloc(void *ptr, size_t size)` – Reallocate existing block
- `mm_free(void *ptr)` – Free allocated memory
- `mm_init()` – Initialise memory pool and metadata
- Choose between **First Fit**, **Best Fit**, and **Worst Fit** via config
- Compact free list embedded directly into memory blocks

---

## How It Works

- Allocates memory from a fixed-size heap array (simulated via `sbrk` or static block)
- Each memory block contains a **header** storing metadata (e.g. size, allocation status)
- Free blocks are linked together in a **singly-linked embedded list**
- Allocation strategies scan the free list differently:
  - **First Fit**: Allocates the first block that fits
  - **Best Fit**: Allocates the smallest sufficient block
  - **Worst Fit**: Allocates the largest possible block
