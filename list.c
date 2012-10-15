#include <stdio.h>
#include <stdlib.h>

#include "list.h"

LIST *
list_create(void)
{
  LIST *node = malloc(sizeof(LIST));
  node->prev = NULL;
  node->next = NULL;
  node->data = NULL;

  return node;
}

LIST *
list_append(LIST *list, void *data)
{
  LIST *new, *last;

  new = list_create();
  new->data = data;

  if(list) {
    last = list_last(list);
    last->next = new;
    new->prev = last;

    return list;
  }

  return new;
}

unsigned int
list_length(LIST *list)
{
  unsigned int length = 0;
  LIST *head = list_first(list);

  while(head) {
    length++;
    head = head->next;
  }

  return length;
}

LIST *
list_first(LIST *list)
{
  if(!list)
    return list;

  while(list->prev)
    list = list->prev;

  return list;
}

LIST *
list_last(LIST *list)
{
  while(list->next)
    list = list->next;

  return list;
}

LIST *
list_remove(LIST *list, void *data)
{
  LIST *tmp = list;

  while(tmp) {
    if(tmp->data != data) {
      tmp = tmp->next;
      continue;
    }

    if(tmp->prev)
      tmp->prev->next = tmp->next;
    if(tmp->next)
      tmp->next->prev = tmp->prev;

    if(tmp == list)
      list = list->next;

    free(tmp);
    break;
  }

  return list;
}
