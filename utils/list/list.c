#include "list.h"

void list_add(list_head_t *list, list_node_t *node)
{
    list_node_t *curr_head;
    do
    {
        curr_head = atomic_load_explicit(&list->head.next, memory_order_acquire);
        atomic_store_explicit(&node->next, curr_head, memory_order_relaxed);
    } while (!atomic_compare_exchange_weak_explicit(&list->head.next, &curr_head, node, memory_order_release, memory_order_acquire));
}

list_node_t *list_pop_head(list_head_t *list)
{
    list_node_t *node;
    list_node_t *next_node;
    do
    {
        node = atomic_load_explicit(&list->head.next, memory_order_acquire);
        if (node == &list->head)
        {
            return NULL;
        }
        next_node = atomic_load_explicit(&node->next, memory_order_relaxed);
    } while (!atomic_compare_exchange_weak_explicit(&list->head.next, &node, next_node, memory_order_release, memory_order_acquire));

    return node;
}

uint32_t list_size(list_head_t *list)
{
    uint32_t     count = 0;
    list_node_t *pos   = atomic_load_explicit(&list->head.next, memory_order_acquire);
    while (pos != &list->head)
    {
        count++;
        pos = atomic_load_explicit(&pos->next, memory_order_relaxed);
    }
    return count;
}
