#include <stdlib.h>
#include <string.h>
#include "linkedList.h"

LinkedList *linkedList_init() {
    return calloc(1, sizeof(LinkedList));
}

ssize_t linkedList_push(LinkedList *head, void *data, size_t size) {
    if (head == NULL)
        return -1;

    if (head->data != NULL) {
        while (head->next != NULL)
            head = head->next;

        head->next = calloc(1, sizeof(LinkedList));
        head->next->data = calloc(size, sizeof(char));
        memcpy(head->next->data, data, size);

    } else {
        head->data = calloc(size, sizeof(char));
        memcpy(head->data, data, size);
    }

    return size;
}

ssize_t linkedList_pop(LinkedList *head, void *buf, size_t size) {
    LinkedList *tmp;

    if (head == NULL || head->data == NULL)
        return -1;

    memcpy(buf, head->data, size);
    free(head->data);
    head->data = NULL;
    if (head->next != NULL) {
        head->data = head->next->data;
        tmp = head->next;
        head->next = head->next->next;
        free(tmp);
    }

    return (ssize_t) size;
}

void *linkedList_getIndexOf(LinkedList *head, int index) {
    int i = 0;

    while (i != index && head != NULL) {
        i++;
        head = head->next;
    }

    if (head != NULL)
        return head->data;

    return NULL;
}

int linkedList_getLength(LinkedList *head) {
    int i = 0;

    if (head == NULL || head->data == NULL)
        return i;

    while (head != NULL) {
        head = head->next;
        i++;
    }

    return i;
}

void linkedList_free(LinkedList *head) {
    LinkedList *prev;

    while (head != NULL) {
        prev = head;
        if (prev->data != NULL)
            free(prev->data);
        head = head->next;
        free(prev);
    }
}