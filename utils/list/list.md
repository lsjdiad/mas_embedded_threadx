# Lock-Free Singly-Linked List

基于 CAS 的无锁单向链表，适用于嵌入式 RTOS 环境下的**多生产者单消费者 (MPSC)** 数据管道。

## 数据结构

### `list_node_t` — 链表节点

| 字段 | 类型 | 说明 |
|------|------|------|
| `next` | `_Atomic(list_node_t *)` | 原子指针，指向下一个节点 |
| `size` | `size_t` | 节点所在结构体总字节数，用于类型安全检查 |

> 必须嵌入在用户结构体中，且作为第一个字段（`offsetof = 0`）：
> ```c
> typedef struct
> {
>     list_node_t node;   // ← 第一个字段
>     uint32_t    id;
>     uint32_t    data;
> } my_item_t;
> ```

### `list_head_t` — 链表头（哨兵）

```c
typedef struct
{
    list_node_t head;   // 哨兵节点：head.next → 首节点 | &head（空表）
} list_head_t;
```
>```
>空链表:            非空链表:
>┌─────────┐        ┌─────────┐  ┌──────┐   ┌──────┐
>│  head   │        │  head   │  │ node2│   │ node1│
>│ .next ──┼→ &head │ .next ───→ │ .next┼─→ │ .next┼──→ &>head
>└─────────┘        └─────────┘  └──────┘   └──────┘
>```
> 空链表时 `head.next` 指向自身，既是哨兵也是判空依据。


## API 说明


```c
static inline void list_init(list_head_t *list);
```
```c
void list_add(list_head_t *list, list_node_t *node);
```
```c
list_node_t *list_pop_head(list_head_t *list);
```
```c
uint32_t list_size(list_head_t *list);
```
```c
#define list_foreach_entry(list, type, member, fn, arg)
```

## 使用示例

### 基本操作

```c
#include "list.h"

typedef struct
{
    list_node_t node;   // 必须是第一个字段
    uint32_t    value;
} item_t;

void demo(void)
{
    list_head_t queue;
    list_init(&queue);               // 初始化

    item_t a = { .value = 10 };
    item_t b = { .value = 20 };
    item_t c = { .value = 30 };

    a.node.size = sizeof(item_t);    // size 用于类型检查
    b.node.size = sizeof(item_t);
    c.node.size = sizeof(item_t);

    list_add(&queue, &a.node);       // 入队: a
    list_add(&queue, &b.node);       // 入队: b → a
    list_add(&queue, &c.node);       // 入队: c → b → a

    // 出队 (LIFO): c, b, a
    item_t *p;
    while ((p = (item_t *)list_pop_head(&queue)) != NULL)
    {
        printf("value: %lu\n", (unsigned long)p->value);
    }
}
```

### 遍历查找

```c
// 回调直接接收 item_t *，无需手动转换
int print_item(item_t *item, void *arg)
{
    (void)arg;
    printf("value: %lu\n", (unsigned long)item->value);
    return 0;
}

// 调用
list_foreach_entry(&queue, item_t, node, print_item, NULL);
```

### 中断 → 线程 MPSC 管道

```c
static list_head_t g_cmd_queue;

// 中断上下文（生产者）
void ISR_Handler(void)
{
    cmd_item_t *cmd = get_cmd_from_isr();
    cmd->node.size = sizeof(cmd_item_t);
    list_add(&g_cmd_queue, &cmd->node);   // 无锁，安全
}

// 线程上下文（消费者）
void CMD_Task(void)
{
    while (1)
    {
        cmd_item_t *cmd;
        while ((cmd = (cmd_item_t *)list_pop_head(&g_cmd_queue)) != NULL)
        {
            process_cmd(cmd);
        }
        tx_thread_sleep(1);
    }
}
```

---

## 并发模型

### CAS 无锁操作原理

`list_add` 和 `list_pop_head` 均使用 CAS（Compare-And-Swap）循环：

```
list_add:                              list_pop_head:
┌──────────────────────┐               ┌──────────────────────┐
│ 1. read head.next    │               │ 1. read head.next    │
│ 2. node.next = head  │               │ 2. if sentinel → NULL│
│ 3. CAS(head, 1, node)│               │ 3. read node.next    │
│    success → done    │               │ 4. CAS(head, node,   │
│    retry  → goto 1   │               │         node.next)   │
└──────────────────────┘               │    success → done    │
                                       │    retry  → goto 1   │
                                       └──────────────────────┘
```

## 注意事项

1. **节点唯一性** — 同一个 `list_node_t` 在弹出前不能再次 `list_add`，否则链表成环，导致后续遍历死循环 → HardFault。

2. **`node.size` 必须设置** — 用于 `list_foreach_entry` 的类型安全检查。
   ```c
   //建议在 `list_add` 前赋值：
   item->node.size = sizeof(item_t);
   ```

3. **`list_node_t` 必须是第一个字段** — `list_foreach_entry` 通过 `offsetof` 反推外层结构体地址，依赖此约定。

4. **消费者唯一** — `list_pop_head` 内部 CAS 循环假定只有一个消费者。多消费者场景需外部加锁。

5. **遍历中不能修改链表** — `list_foreach` / `list_foreach_entry` 回调内禁止调用 `list_add` / `list_pop_head`。

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| v1.0 | 2026-05-4 | 初始版本 |