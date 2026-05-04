/*
 * @Author: laladuduqq 2807523947@qq.com
 * @Date: 2026-05-03 23:35:52
 * @LastEditors: laladuduqq 2807523947@qq.com
 * @LastEditTime: 2026-05-04 11:33:37
 * @FilePath: /mas_embedded_threadx/utils/list/list.h
 * @Description:
 */
#ifndef _LIST_H_
#define _LIST_H_

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

// 数据类型

/**
 * @brief 链表节点基类。
 * @note  必须嵌入在用户结构体中，且应为第一个字段（list_foreach_entry 依赖此约定）。
 */
typedef struct list_node
{
    _Atomic(struct list_node *) next; /* 原子指针，指向后继节点 */
    size_t                      size; /* 外覆结构体的 sizeof，用于类型安全检查 */
} list_node_t;

/**
 * @brief 链表头（哨兵节点容器）。
 *
 * head 是内嵌的哨兵节点，不存储用户数据。
 * 空链表: head.next == &head
 * 非空:   head.next → 首节点
 */
typedef struct
{
    list_node_t head; /* 哨兵节点 */
} list_head_t;

/**
 * @brief 初始化链表，将哨兵节点的 next 指向自身，表示空链表。
 * @param list 链表头指针
 * @note  必须在任何其他操作前调用。重复初始化是安全的（幂等）。
 */
static inline void list_init(list_head_t *list) { atomic_store_explicit(&list->head.next, &list->head, memory_order_relaxed); }

/**
 * @brief 头插法添加节点。
 * @param list 链表头指针
 * @param node 待插入节点的指针（node->size 应预先设置为 sizeof(外覆结构体)）
 * @note  中断安全，可在中断上下文中调用，不会阻塞
 * @note  同一节点在弹出前不可再次 list_add，否则链表成环 → HardFault。
 * @note  同一节点不可被多个生产者同时 add。
 */
void list_add(list_head_t *list, list_node_t *node);

/**
 * @brief  弹出头部节点,移除并返回链表首节点。空链表返回 NULL。
 * @param  list 链表头指针
 * @return 弹出的 list_node_t 指针，空链表返回 NULL
 * @note   仅限单消费者调用。多消费者场景需外部同步。
 */
list_node_t *list_pop_head(list_head_t *list);

/**
 * @brief  获取链表节点数量遍历整个链表计数。
 * @param  list 链表头指针
 * @return 当前链表中的节点数
 * @note   遍历期间链表可能被生产者修改，返回值是近似快照
 */
uint32_t list_size(list_head_t *list);

/**
 * @brief 遍历链表，回调接收外覆结构体指针。
 *
 * 自动通过 offsetof 将内部 list_node_t * 转换为包含它的外覆结构体指针，
 * 回调函数直接接收业务类型，无需手动强制转换。
 *
 * @param list   链表头指针 (list_head_t *)
 * @param type   外覆结构体类型名（如 my_item_t）
 * @param member list_node_t 在外覆结构体中的字段名（如 node）
 * @param fn     回调函数，签名为 int (*)(type *, void *)
 *               - 返回 0 继续遍历
 *               - 返回非 0 提前终止
 * @param arg    透传给回调的 void * 参数，可为 NULL
 * @note  回调内不可调用 list_add / list_pop_head 修改当前链表。
 * @note 外覆结构体中 list_node_t 必须是第一个字段（offsetof(type, member) == 0）。
 */
#define list_foreach_entry(list, type, member, fn, arg)                                                                                              \
    do                                                                                                                                               \
    {                                                                                                                                                \
        list_node_t *__pos = atomic_load_explicit(&(list)->head.next, memory_order_acquire);                                                         \
        while (__pos != &(list)->head)                                                                                                               \
        {                                                                                                                                            \
            type *__entry = (type *)((char *)__pos - offsetof(type, member));                                                                        \
            if (fn(__entry, arg) != 0) break;                                                                                                        \
            __pos = atomic_load_explicit(&__pos->next, memory_order_relaxed);                                                                        \
        }                                                                                                                                            \
    } while (0)

#endif /* _LIST_H_ */
