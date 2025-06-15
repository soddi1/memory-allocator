#include "core_mem.h"
#include "mm_lib.h"
#include "utils.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// -------- Macros defined for the allocator --------
const size_t Allignment = 8;
const int magic = 543456;

// --------- Definitions of the headers ---------
typedef struct __node_t
{
    size_t size;
    struct __node_t *next;
} node_t;

typedef struct
{
    size_t size;
    int magic;
} header_t;

node_t *head;

// --------- Helper function declarations ---------
void Delete_node(node_t* node);
size_t allign(size_t size);
void coalescing();
void Add_node(node_t* new_node);
void Delete_node(node_t* node);
void *first_fit(size_t size);
void *best_fit(size_t size);
void *worst_fit(size_t size);
void *split_node(node_t* node, size_t size);

// --------- Function Definitions ---------
void mm_init()
{
    head = cm_sbrk(sizeof(node_t));
    head->size=(sizeof(node_t));
    head->next = NULL;
}

void *mm_malloc(size_t size)
{
    size_t size_plus_header = allign(size);
    void* ptr = NULL;

    char *search_scheme = getenv("SEARCH_SCHEME");
    if(search_scheme==NULL)
    {
        perror("Wrong input");
    }
    else if(strcmp(search_scheme, "FIRST_FIT") == 0)
    {
       ptr = first_fit(size_plus_header);
    }
    else if(strcmp(search_scheme, "BEST_FIT") == 0)
    {
        ptr = best_fit(size_plus_header);
    }
    else if(strcmp(search_scheme, "WORST_FIT") == 0)
    {
        ptr = worst_fit(size_plus_header);
    }

    node_t* new = head;
    int found = 0;
    while(new!=NULL)
    {
        if (ptr == new)
            found = 1;

        new = new->next;
    }

    if(found == 0)
        return ptr;

    node_t *node = ptr;
    void* temp = NULL;
    if (node->size >= size_plus_header + sizeof(node_t))
    {
        temp = split_node(node, size_plus_header);
    }

    Delete_node(node);

    node->size -= sizeof(header_t);

    return temp;
}

void mm_free(void* ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    else
    {
        header_t* header = (header_t*)PTR_SUB(ptr, sizeof(header_t));
        header->size += sizeof(header_t);
        node_t* node = (node_t*)header;
        node->next = head;
        Add_node(node);
        coalescing();
    }
}

void* mm_realloc(void* ptr, size_t size)
{
    if(ptr == NULL)
    {
        return mm_malloc(size);
    }
    if(size == 0) 
    {
        mm_free(ptr);
        return NULL;
    }

    void* new_mem = mm_malloc(size);
    if (new_mem == NULL)
    {
        return NULL;
    }

    header_t* new_header = (header_t*)new_mem - 1;
    header_t* old_header = (header_t*)ptr - 1;

    size_t new_size = new_header->size;
    size_t old_size = old_header->size;

    size_t copy_size = (old_size < new_size) ? old_size : new_size;

    memcpy(new_mem, ptr, copy_size);

    mm_free(ptr);

    return new_mem;
}

void *first_fit(size_t size)
{
    node_t* node = NULL;
    node_t *current = head;

    while(current)
    {
        if (current->size >= size + sizeof(header_t))
        {
            return current;
        }
            
        current = current->next;
    }

    if(node == NULL) 
    {
        current = cm_sbrk(size);
        header_t* header = (header_t*)current;
        header->size = size - sizeof(header_t);
        header->magic = magic;
        current = PTR_ADD(current, sizeof(header_t));
        node = current;
        return node;
    } 

    return NULL;
}

void *best_fit(size_t size)
{
    node_t *current = head;
    node_t *node = NULL;

    while (current)
    {
        if (current->size >= size + sizeof(header_t))
        {
            if (node == NULL || current->size < node->size)
            {
                node = current;
            }
        }

        current = current->next;
    }

    if(node == NULL) 
    {
        current = cm_sbrk(size);
        header_t* header = (header_t*)current;
        header->size = size - sizeof(header_t);
        header->magic = magic;
        current = PTR_ADD(current, sizeof(header_t));
        node = current;
    } 
 
    return node;
}

void *worst_fit(size_t size)
{
    node_t *current = head;
    node_t *node = NULL;

    while (current)
    {
        if (current->size >= size + sizeof(header_t))
        {
            if (node == NULL || current->size > node->size)
            {
                node = current;
            }
        }

        current = current->next;
    }

    if(node == NULL)
    {
        current = cm_sbrk(size);
        header_t* header = (header_t*)current;
        header->size = size - sizeof(header_t);
        header->magic = magic;
        current = PTR_ADD(current, sizeof(header_t));
        node = current;
    } 
 
    return node;
}

size_t allign(size_t size)
{
    size_t new_size = size + sizeof(header_t);
    if (new_size % Allignment != 0)
    {
        new_size = (new_size + (Allignment - 1)) & ~(Allignment - 1);
    }
    return new_size;
}

void Add_node(node_t* new_node)
{
    node_t* current = head;
    node_t* previous = NULL;

    while(current!=NULL && new_node>current)
    {
        previous = current;
        current = current->next;
    }
    if(previous==NULL)
    {
        new_node->next = head;
        head = new_node;
    }
    else
    {
        previous->next = new_node;
        new_node->next = current;
    }
    coalescing();
}

void Delete_node(node_t* node)
{
    node_t* current = head;
    node_t* previous = NULL;

    while(current!=NULL && current!=node)
    {
        previous = current;
        current = current->next;
    }
    if(current==NULL)
    {
        printf("Node not found\n");
        return;
    }
    if(previous==NULL)
    {
        head = current->next;
    }
    else
    {
        previous->next = current->next;
    }
    coalescing();
}

void coalescing() {
    node_t *ptr = head;

    while (ptr != NULL && ptr->next != NULL) {
        if (PTR_ADD(ptr, ptr->size) == ptr->next) {
            ptr->size += ptr->next->size;
            ptr->next = ptr->next->next;
        }
        ptr = ptr->next;
    }
}

void* split_node(node_t* node, size_t size)
{
    header_t* temp = (header_t*)node;
    size_t remaining_space = node->size - size - sizeof(header_t);
    if(remaining_space > sizeof(node_t))
    {
        node_t *new_node = PTR_ADD(node, size + sizeof(header_t));
        new_node->size = remaining_space;
        Add_node(new_node);
        node->size -= new_node->size;
    }
    return PTR_ADD(temp, sizeof(header_t));
}

