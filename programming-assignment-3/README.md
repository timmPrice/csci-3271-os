# Paging Simulation Project

This program simulates a memory-management system for a single process that can be configured with several different page replacement algorithms. The simulator loads a sequence of virtual memory accesses from a data file, constructs a page table manager object that uses the requested algorithm, and then asks the page table manager to translate each memory address in the sequence. The page table manager counts the number of page faults that occur (assuming it is configured with a physical memory size smaller than the virtual memory size), and the simulator prints out this total at the end.

## Compiling the program

This program is set up as a CMake project, which allows it to be compiled in either Windows or Linux as long as you have CMake installed.

On Windows, launch Visual Studio and choose the "Open Folder" option (rather than "Open Project"), then select the root paging-simulation folder that contains CMakeLists.txt and CMakePresets.json. You should then be able to choose the "Windows Debug" build configuration and build the project. The compiled executable will be placed in `out\build\windows-debug\src`, and will be named `paging_simulation.exe`.

On Linux, open a terminal in the paging-simulation directory and run cmake with the linux-debug preset:

```console
~/paging-simulation$ cmake --preset linux-debug
```

This will create a directory named build-Debug, which you can then instruct CMake to compile in:

```console
~/paging-simulation$ cmake --build build-Debug
```

The compiled executable will be placed in `build-Debug/src` and will be named `paging_simulation`.

## Running the simulation

When running the program, it expects four command-line arguments: The page size in addresses per page, the physical memory size in frames, the paging algorithm to use (one of FIFO, LRU, RANDOM, CLOCK, OPT), and a path to the data file containing the memory trace to use. Since all of the data files provided with this project use memory addresses from 0-1023 (they are traces of sort algorithms run on an array of size 1024), the page size must be less than 1024 for the simulation to work. As an example, here is a typical invocation of the program with pages that contain 16 addresses and 10 frames of physical memory, assuming it has been compiled with CMake on Linux:

```console
~/paging-simulation/build-Debug$ src/paging_simulation 16 10 LRU ../data/heapsort.trace
```

To help make sure your memory-size arguments are reasonable, the program will print out the number of pages and frames that it has been configured with before starting the situation, and it will print a warning if your chosen page size results in only 1 page.

## Pager Objects

This program uses a form of polymorphism in C to represent the page table managers: The simulator's main loop interacts with a `pager_interface`, which is a `struct` that contains several function pointers representing interface methods. When the program starts up, it creates the `pager_interface` by calling one of the 5 different `pager_new` functions: `fifo_pager_new`, `lru_pager_new`, `clock_pager_new`, `random_pager_new`, or `opt_pager_new`. Each `new` function binds a different set of "method" functions to the `pager_interface`'s function pointers, based on which algorithm is desired.

The pager interface is defined in pager.h, and the comments on each function pointer in the struct describe the intended use of each "method." The "method" functions, like other object-oriented C functions, always take as their first argument a pointer to the `pager_interface` object that called them, which can be used like the `this` pointer or `self` reference in other languages. The pager interface also contains a pointer to a `common_pager_state`, which contains some "instance variables" that all pager objects will need to use, and a `void` pointer that is intended to be used by each implementation of the interface to point to its algorithm-specific instance variables.

Pager.h also contains some declarations of "default" methods, which are implemented in pager.c. These are intended to be used like base class methods in other languages: A pager implementation that does not need its own algorithm-specific logic for a method can use the default method instead. For example, most pager implementations won't need to change the behavior of the `get_num_page_faults` method, so they can set `get_num_page_faults` to `default_get_num_page_faults` to use the default implementation.

The file dummy_pager.c shows an example of how to implement a page table manager using the pager interface. It defines a "state" structure that it will use to store its "algorithm-specific" instance variables, which in this case is just one, an int named `access_count`:

```c
typedef struct dummy_pager_state {
    int access_count;
} dummy_pager_state;
```

It then defines implementations of the `translate` and `handle_page_fault` methods, called `dummy_translate` and `dummy_handle_page_fault`. In the `dummy_pager_new` method, it initializes the function pointers in the `pager_interface` to point to these methods, and uses the default methods for `get_num_page_faults` and `check_table_contains`. It also sets the `state` pointer (the `void*` intended for algorithm-specific state) to an instance of `dummy_pager_state`.

```c
pager_interface* dummy_pager_new(const memory_constants* memory_params) {
    pager_interface* self = checked_malloc(sizeof(*self));
    dummy_pager_state* state = checked_malloc(sizeof(*state));
    common_pager_state* common_state = common_state_new(memory_params);

    self->translate = dummy_translate;
    self->handle_page_fault = dummy_handle_page_fault;
    self->get_num_page_faults = default_get_num_page_faults;
    self->check_table_contains = default_check_table_contains;
    self->common_state = common_state;
    self->state = state;
    return self;
}
```

The `dummy_translate` method shows a way of simulating inheritance in C. After updating the `access_count` instance variable in `self->state`, it calls the `default_translate` function to "inherit" its behavior without needing to rewrite it. This is similar to a call to `base.Translate` in C#.

```c
int dummy_translate(pager_interface* self, int virtual_address) {
    dummy_pager_state* my_state = self->state;
    my_state->access_count++;
    return default_translate(self, virtual_address);
}
```

## List and Vector

This project comes with two general-purpose data structures that have been implemented for you, in the `src/util` directory. Both use the object-oriented style of C code, in which every function that acts on a `list` or `vector` takes as its first parameter a pointer to the `list` or `vector` it is being called on. Both are also defined to store non-owning `void` pointers to arbitrary data, so it is up to the user of these data structures to allocate and free the memory for the items in the list or vector.

The `list` data structure is a doubly-linked list that can be accessed from either the head (first) or the tail (last). Its interface is documented with comments in list.h. As an example, here is some code that creates a list, adds some `int` values to it, and then removes them in FIFO order. Note that the `int` values are allocated with `malloc` before being placed on the list, and then freed with `free` after being removed from the list.

```c
list* my_list = list_new();
int* item_one = malloc(sizeof(int));
*item_one = 10;
int* item_two = malloc(sizeof(int));
*item_two = 20;
int* item_three = malloc(sizeof(int));
*item_three = 30;
if(list_add_first(my_list, item_one) != 0) {
    printf("Error: Could not add item_one to list!\n");
    exit(-1);
}
if(list_add_first(my_list, item_two) != 0) {
    printf("Error: Could not add item_two to list!\n");
    exit(-1);
}
if(list_add_first(my_list, item_three) != 0) {
    printf("Error: Could not add item_three to list!\n");
    exit(-1);
}
int length = list_length(my_list);
printf("Length of list: %d\n", length);
while(list_length(my_list) > 0) {
    void* list_item;
    if(list_remove_last(my_list, &list_item) != 0) {
        printf("Error: Could not remove an item from the list!\n");
        exit(-1);
    }
    printf("Dequeued %d from the list", *((int*)list_item));
    free(list_item);
}
list_free(my_list);
```

The `vector` data structure is a variable-size array that grows automatically to accommodate new items that are added with `push_back`, and allows random access to any existing index with `get` and `set`. Its interface is documented with comments in in vector.h. As an example, here some code that creates a vector, adds some `int` values to it, displays every other value in the array, and then frees all the vector's memory.

```c
vector* my_vector = vector_new();
for(int item_count = 0; item_count < 10; item_count++) {
    int* item = malloc(sizeof(int));
    *item = item_count * 2;
    if(vector_push_back(my_vector, item) != 0) {
        printf("Error: could not add item %d to the vector!\n", item_count);
        exit(-1);
    }
}
for(int i = 0; i < vector_size(my_vector); i += 2) {
    void* vector_item;
    if(vector_get(my_vector, i, &vector_item) != 0) {
        printf("Error: Could not get element %d from vector!\n", i);
        exit(-1);
    }
    printf("my_vector[%d] is %d\n", i, *((int*)vector_item));
}
while(vector_size(my_vector) > 0) {
    void* vector_item;
    if(vector_pop_back(my_vector, &vector_item) == 0) {
        free(vector_item);
    }
}
vector_free(my_vector);
```
