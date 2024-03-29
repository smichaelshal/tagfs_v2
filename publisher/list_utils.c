#include "publisher.h"
#include "../utils/utils.h"


// ----- dentry_list -----

#define DEFAULT_TEXT_CURSOR "CURSOR"
#define DEFAULT_TEXT_REG "REG_NULL"

struct dentry_list *add_dentry_list(struct list_head *list, struct dentry_list *dentry_list){
    list_add(&dentry_list->child, list);
    return dentry_list;
}

struct dentry_list *add_dentry_cursor(struct list_head *list){
    struct dentry_list *cursor;
    cursor = alloc_dentry_list();
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag = CURSOR_MODE;
    list_add(&cursor->child, list);
    return cursor;
}

struct dentry_list *get_next_dentry_list(struct dentry_list *cursor, struct list_head *head){
    struct list_head *next;

    next = cursor->child.next;
    if(next == head)
        return NULL;

    return list_entry(next, struct dentry_list, child);
}


struct dentry_list *get_current_dentry_list(struct dentry_list *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_dentry_list(cursor, head);
    return cursor;
}

void update_dentry_list_cursor(struct dentry_list *cursor, struct dentry_list *next){
    list_move(&cursor->child, &next->child);
}

struct dentry_list *get_current_update_dentry(struct dentry_list *cursor, struct list_head *head, spinlock_t *lock){
    struct dentry_list *current_dentry_list;
    if(lock)
        spin_lock(lock);
    current_dentry_list = get_current_dentry_list(cursor, head);
    if(!current_dentry_list)
        goto out;
    
    update_dentry_list_cursor(cursor, current_dentry_list);
out:
    if(lock)
        spin_unlock(lock);
    return current_dentry_list;
}

// ----- vbranch -----

struct vbranch *add_vbranch(struct list_head *list, struct vbranch *vbranch){
    pr_info("vbranch addr: %x\n", vbranch);
    if(!vbranch->name)
        vbranch->name = dup_name(DEFAULT_TEXT_REG);
    list_add_tail(&vbranch->child, list);
    return vbranch;
}

struct vbranch *add_vbranch_cursor(struct list_head *list){
    struct vbranch *cursor;
    cursor = alloc_vbranch();
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag = CURSOR_MODE;
    cursor->name = dup_name(DEFAULT_TEXT_CURSOR);
    list_add(&cursor->child, list);
    pr_info("cursor vbranch addr: %x\n", cursor);
    return cursor;
}

struct vbranch *get_next_vbranch(struct vbranch *cursor, struct list_head *head){
    struct list_head *next;

    next = cursor->child.next;
    if(next == head)
        return NULL;

    return list_entry(next, struct vbranch, child);
}

struct vbranch *get_prev_vbranch(struct vbranch *cursor, struct list_head *head){
    struct list_head *prev;

    prev = cursor->child.prev;
    if(prev == head)
        return NULL;

    return list_entry(prev, struct vbranch, child);
}

struct vbranch *get_current_vbranch(struct vbranch *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE){
        pr_info("y0\n");
        cursor = get_next_vbranch(cursor, head);
    }
    return cursor;
}

struct vbranch *get_current_vbranch_prev(struct vbranch *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE){
        pr_info("y1\n");
        cursor = get_prev_vbranch(cursor, head);
    }
    return cursor;
}

void update_vbranch_cursor(struct vbranch *cursor, struct vbranch *next){
    list_move(&cursor->child, &next->child);
}

struct vbranch *get_current_update_vbranch(struct vbranch *cursor, struct list_head *head, spinlock_t *lock){
    struct vbranch *current_vbranch;
    if(lock)
        spin_lock(lock);
    current_vbranch = get_current_vbranch(cursor, head);
    if(!current_vbranch)
        goto out;
    
    update_vbranch_cursor(cursor, current_vbranch);
out:
    if(lock)
        spin_unlock(lock);
    return current_vbranch;
}


// ----- db_tag -----

struct db_tag *add_db_tag(struct list_head *list, struct db_tag *db_tag){
    pr_info("add_db_tag addr: %x\n", db_tag);
    if(!db_tag->name)
        db_tag->name = dup_name(DEFAULT_TEXT_REG);
    list_add(&db_tag->child, list);
    return db_tag;
}

struct db_tag *add_db_tag_cursor(struct list_head *list){
    struct db_tag *cursor;
    cursor = alloc_db_tag();
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag = CURSOR_MODE;
    cursor->name = dup_name(DEFAULT_TEXT_CURSOR);
    list_add(&cursor->child, list);
    pr_info("cursor add_db_tag addr: %x\n", cursor);
    return cursor;
}

struct db_tag *get_next_db_tag(struct db_tag *cursor, struct list_head *head){
    struct list_head *next;

    next = cursor->child.next;
    if(next == head)
        return NULL;

    return list_entry(next, struct db_tag, child);
}

struct db_tag *get_current_db_tag(struct db_tag *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_db_tag(cursor, head);
    return cursor;
}

void update_db_tag_cursor(struct db_tag *cursor, struct db_tag *next){
    list_move(&cursor->child, &next->child);
}

struct db_tag *get_current_update_db_tag(struct db_tag *cursor, struct list_head *head, spinlock_t *lock){
    struct db_tag *current_db_tag;
    if(lock)
        spin_lock(lock);
    current_db_tag = get_current_db_tag(cursor, head);
    if(!current_db_tag)
        goto out;
    
    update_db_tag_cursor(cursor, current_db_tag);
out:
    if(lock)
        spin_unlock(lock);
    return current_db_tag;
}


bool is_vbranch_empty(struct vbranch *vbranch){
    return list_empty(&vbranch->dentries);
}


bool is_db_tag_empty(struct db_tag *db_tag){
    return list_empty(&db_tag->vbranchs);
}


bool is_vtag_empty(struct vtag *vtag){ // ???
    return list_empty(&vtag->db_tags);
}

void delete_db_tag(struct db_tag *db_tag){
    list_del(&db_tag->child);
}

void delete_dentry_list(struct dentry_list *dentry_list){
    list_del(&dentry_list->child);
}

void delete_vbranch(struct vbranch *vbranch){
    list_del(&vbranch->child);
}

void move_vbranch_cursor(struct vbranch *cursor, struct list_head *list){ // <<<
    list_move(&cursor->child, list);
}