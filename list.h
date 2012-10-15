typedef struct list_t {
  void *data;
  struct list_t *prev;
  struct list_t *next;
} LIST;

LIST *list_append(LIST *list, void *data);
LIST *list_create(void);
unsigned int list_length(LIST *list);
LIST *list_first(LIST *list);
LIST *list_last(LIST *list);
LIST *list_remove(LIST *list, void *data);
