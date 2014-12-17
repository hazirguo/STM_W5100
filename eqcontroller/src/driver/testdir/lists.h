/*****************************************************************
 *                     LISTS.H
 *
 *  PURPOSE: Header file for LISTS.C
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *
 ******************************************************************/

#ifndef LISTS_H

#define LISTS_H
#define MAXSTRING 256

struct Element {
    struct Element *next;
    struct Element *prev;
    union {
	void *object;		/* Object that this points to */
	int value;
    }
     Obj;
#define Object Obj.object
#define Value  Obj.value
};

typedef struct Element Element;

struct List {
    struct Element *next;	/* Next list in a list of lists */
    struct Element *prev;	/* Prev list in a list of lists */

    char name[MAXSTRING];
    int number;
    struct Element *head;
    struct Element *tail;

    int maxNumber;		/* Max number allowed in list. */
    /* if < 0, no limit set. */

};

typedef struct List List;

/* Returns from calls to routines that add to a list. */
#define LIST_OK 0
#define LIST_ERROR_FULL 1

#ifndef LISTS_C
extern struct List *createList(char *);
extern struct Element *createElement(void *);
extern int addElement(struct List *, struct Element *);
extern int addElementBefore(struct List *, struct Element *, struct Element *);
extern struct Element *deleteElement(struct List *, struct Element *);
extern void destroyList(struct List *);
extern void destroyElement(struct Element *);
extern struct Element *removeElement(struct List *);
#endif

#endif				/* LISTS_H */
