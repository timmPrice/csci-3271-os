#include "list.h"
#include "checked_malloc.h"

#include <stdlib.h>
#include <stdio.h>

#define SUCCESS 0
#define FAILURE -1

typedef struct list_node {
    void* data;
    struct list_node* next;
    struct list_node* previous;
} list_node;

struct list {
    list_node* head;
    list_node* tail;
    int length;
};

list* list_new() {
    list* new_list = checked_malloc(sizeof(*new_list));
    new_list->head = NULL;
    new_list->tail = NULL;
    new_list->length = 0;
    return new_list;
}

int list_free(list* list) {
    if(list == NULL || list->length > 0) {
        return FAILURE;
    }
    free(list);
    return SUCCESS;
}

int list_add_first(list* list, void* item) {
    if(list == NULL) {
        return FAILURE;
    }

    list_node* new_node = checked_malloc(sizeof(*new_node));
    new_node->data = item;
    new_node->next = list->head;
    new_node->previous = NULL;

    if (list->head != NULL) {
        list->head->previous = new_node;
    }
    list->head = new_node;
    //If this is the first item in the list, it will be both head and tail
    if(list->tail == NULL) {
        list->tail = new_node;
    }
    list->length++;

    return SUCCESS;
}

int list_add_last(list* list, void* item) {
    if(list == NULL) {
        return FAILURE;
    }
    list_node* new_node = checked_malloc(sizeof(*new_node));
    new_node->data = item;
    new_node->next = NULL;
    new_node->previous = list->tail;

    if(list->tail != NULL) {
        list->tail->next = new_node;
    }
    list->tail = new_node;
    //If this is the first item in the list, it will be both head and tail
    if(list->head == NULL) {
        list->head = new_node;
    }
    list->length++;
    return SUCCESS;
}

int list_length(const list* list) {
    if(list == NULL) {
        return FAILURE;
    }
    return list->length;
}

int list_remove_first(list* list, void** item) {
    if(list == NULL || item == NULL) {
        return FAILURE;
    }
    if(list->head == NULL) {
        *item = NULL;
        return FAILURE;
    }
    *item = list->head->data;
    // If there was only one item in the list, it will now be empty
    // Otherwise, make head->next the new head
    if(list->head->next == NULL) {
        list->tail = NULL;
    } else {
        list->head->next->previous = NULL;
    }
    list_node* old_head = list->head;
    list->head = old_head->next;
    free(old_head);
    list->length--;

    return SUCCESS;
}

int list_remove_last(list* list, void** item) {
    if(list == NULL || item == NULL) {
        return FAILURE;
    }
    if(list->tail == NULL) {
        *item = NULL;
        return FAILURE;
    }
    *item = list->tail->data;
    // If there was only one item in the list, it will now be empty
    // Otherwise, make tail->previous the new tail
    if(list->tail->previous == NULL) {
        list->head = NULL;
    } else {
        list->tail->previous->next = NULL;
    }
    list_node* old_tail = list->tail;
    list->tail = old_tail->previous;
    free(old_tail);
    list->length--;

    return SUCCESS;
}

int list_delete(list* list, void* item, equals_func is_equal) {
    if(list == NULL || item == NULL || is_equal == NULL) {
        return FAILURE;
    }

    list_node* cur_node = list->head;
    while(cur_node != NULL) {
        if(is_equal(cur_node->data, item)) {
            if(cur_node->next == NULL) {
                // Remove the tail
                list->tail = cur_node->previous;
            } else {
                // Re-point the next node's "previous" around cur_node
                cur_node->next->previous = cur_node->previous;
            }
            if(cur_node->previous == NULL) {
                // Remove the head
                list->head = cur_node->next;
            } else {
                // Re-point the previous node's "next" around cur_node
                cur_node->previous->next = cur_node->next;
            }
            // Since the caller won't get the data pointer back, and might not have a copy of it,
            // we need to delete the data here. This won't work if data is a complex object with
            // its own "x_free" method, but it's the best we can do.
            free(cur_node->data);
            free(cur_node);
            list->length--;
            return SUCCESS;
        }
        cur_node = cur_node->next;
    }
    return FAILURE;
}
