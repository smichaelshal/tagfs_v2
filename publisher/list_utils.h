#ifndef LIST_UTILS_H_
#define LIST_UTILS_H_

#include "publisher.h"


// ----- dentry_list -----

extern struct dentry_list *add_dentry_list(struct list_head *list, struct dentry_list *dentry_list);

extern struct dentry_list *add_dentry_cursor(struct list_head *list);

extern struct dentry_list *get_next_dentry_list(struct dentry_list *cursor, struct list_head *head);

extern struct dentry_list *get_current_dentry_list(struct dentry_list *cursor, struct list_head *head);

extern void update_dentry_list_cursor(struct dentry_list *cursor, struct dentry_list *next);

extern struct dentry_list *get_current_update_dentry(struct dentry_list *cursor, struct list_head *head, spinlock_t *lock);

// ----- vbranch -----

extern struct vbranch *add_vbranch(struct list_head *list, struct vbranch *vbranch);

extern struct vbranch *add_vbranch_cursor(struct list_head *list);

extern struct vbranch *get_next_vbranch(struct vbranch *cursor, struct list_head *head);

extern struct vbranch *get_current_vbranch(struct vbranch *cursor, struct list_head *head);

extern void update_vbranch_cursor(struct vbranch *cursor, struct vbranch *next);

extern struct vbranch *get_current_update_vbranch(struct vbranch *cursor, struct list_head *head, spinlock_t *lock);

// ----- db_tag -----

extern struct db_tag *add_db_tag(struct list_head *list, struct db_tag *db_tag);

extern struct db_tag *add_db_tag_cursor(struct list_head *list);

extern struct db_tag *get_next_db_tag(struct db_tag *cursor, struct list_head *head);

extern struct db_tag *get_current_db_tag(struct db_tag *cursor, struct list_head *head);

extern void update_db_tag_cursor(struct db_tag *cursor, struct db_tag *next);

extern struct db_tag *get_current_update_db_tag(struct db_tag *cursor, struct list_head *head, spinlock_t *lock);

extern bool is_vbranch_empty(struct vbranch *vbranch);

extern bool is_db_tag_empty(struct db_tag *db_tag);

extern bool is_vtag_empty(struct vtag *vtag);

extern void delete_db_tag(struct db_tag *db_tag);

extern void delete_dentry_list(struct dentry_list *dentry_list);

extern void delete_vbranch(struct vbranch *vbranch);

extern void move_vbranch_cursor(struct vbranch *cursor, struct list_head *list);
#endif /* LIST_UTILS_H_ */
