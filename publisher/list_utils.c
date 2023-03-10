#include "publisher.h"


// ----- dentry_list -----


struct dentry_list *add_dentry_list(struct list_head *list, struct dentry_list *dentry_list){
    list_add(&new_dentry_list->child, list);
    return new_dentry_list;
}

struct dentry_list *add_dentry_cursor(struct list_head *list){
    struct dentry_list *cursor;
    cursor = kzalloc(sizeof(struct dentry_list), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add(&cursor->child, list);
    return cursor;
}

struct dentry_list *get_next_dentry_list(struct dentry_list *cursor, struct list_head *head){
    struct list_head *next;
    struct dentry_list *current_dentry_list;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_dentry_list = list_entry(next, struct dentry_list, child);
    return current_dentry_list;
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
    list_add(&new_vbranch->child, list);
    return new_vbranch;
}

struct vbranch *add_vbranch_cursor(struct list_head *list){
    struct vbranch *cursor;
    cursor = kzalloc(sizeof(struct vbranch), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add_tail(&cursor->child, list);
    return cursor;
}

struct vbranch *get_next_vbranch(struct vbranch *cursor, struct list_head *head){
    struct list_head *next;
    struct vbranch *current_vbranch;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_vbranch = list_entry(next, struct vbranch, child);
    return current_vbranch;
}

struct vbranch *get_current_vbranch(struct vbranch *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_vbranch(cursor, head);
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
    list_add(&new_db_tag->child, list);
    return new_db_tag;
}

struct db_tag *add_db_tag_cursor(struct list_head *list){
    struct db_tag *cursor;
    cursor = kzalloc(sizeof(struct db_tag), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add_tail(&cursor->child, list);
    return cursor;
}

struct db_tag *get_next_db_tag(struct db_tag *cursor, struct list_head *head){
    struct list_head *next;
    struct db_tag *current_db_tag;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_db_tag = list_entry(next, struct db_tag, child);
    return current_db_tag;
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
    return list_empty(vbranch->dentries);
}


bool is_db_tag_empty(struct db_tag *db_tag){
    return list_empty(db_tag->vbranchs);
}


bool is_vtag_empty(struct vtag *vtag){ // ???
    return list_empty(vtag->db_tags);
}

void delete_db_tag(struct db_tag *db_tag){
    list_del(db_tag->child);
}

void delete_dentry_list(struct dentry_list *dentry_list){
    list_del(dentry_list->child);
}

void delete_vbranch(struct vbranch *vbranch){
    list_del(vbranch->child);
}

void move_vbranch_cursor(struct vbranch *cursor, struct list_head *list){ // <<<
    list_move(&cursor->child, list);
}