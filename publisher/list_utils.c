#define CURSOR_MODE 1

struct list_number {
    int value;
    struct list_head child;
    long flag;
};

struct list_number *add_item(struct list_head *list, int value){
    /*
    * add item to list.
    * return the new item if success, else return NULL
    * 
    * list_add_tail - the cursor sees the addition of a new item
    * list_add - the cursor does not see the addition of a new item
    */
    struct list_number *new_item;
    new_item = kzalloc(sizeof(struct list_number), GFP_KERNEL);
    if(!new_item)
        return NULL;
    INIT_LIST_HEAD(&new_item->child);
    new_item->value = value;
    list_add_tail(&new_item->child, list);
    return new_item;
}

struct list_number *add_cursor(struct list_head *list){
    /*
    * add cursor to list.
    * return the new cursor if success, else return NULL
    */
    
    struct list_number *cursor;
    cursor = kzalloc(sizeof(struct list_number), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add(&cursor->child, list);
    return cursor;
}

void build_list(struct list_head *list){
    INIT_LIST_HEAD(list);
    add_item(list, 1);
    add_item(list, 2);
    add_item(list, 3);
}

struct list_number *get_next_item(struct list_number *cursor, struct list_head *head){
    /*
    * return the next object after cursor (dont skip cursor)
    */

    struct list_head *next;
    struct list_number *current_item;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_item = list_entry(next, struct list_number, child);
    return current_item;
}

struct list_number *get_current_item(struct list_number *cursor, struct list_head *head){
    /*
    * return the next relvant object after cursor (skip all cursors)
    */

    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_item(cursor, head);
    return cursor;
}

void update_cursor(struct list_number *cursor, struct list_number *next){
    /*
    * update to posion of cursor
    */
    list_move(&cursor->child, &next->child);
}

struct list_number *get_current_update(struct list_number *cursor, struct list_head *head, spinlock_t *lock){
    /*
    * return the next object after cursor,
    * and update the cursor to next poition
    *
    * Because the cursor is updated,
    * the next time you call get_current_item,
    * you get the next value after the current value
    * (When this function returns).
    */
    struct list_number *current_item;
    if(lock)
        spin_lock(lock);
    current_item = get_current_item(cursor, head);
    if(!current_item)
        goto out;
    
    update_cursor(cursor, current_item);
out:
    if(lock)
        spin_unlock(lock);
    return current_item;
}

void test_list(void){
    struct list_head list;
    struct list_number *cursor, *current_item;

    spinlock_t list_lock;
    spin_lock_init(&list_lock);

    build_list(&list);

    cursor = add_cursor(&list);
    current_item = cursor;
    while(1){
        current_item = get_current_update(cursor, &list, &list_lock);
        if(!current_item)
            break;
        pr_info("current value: %d\n", current_item->value);
    }
}