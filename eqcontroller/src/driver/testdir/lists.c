/*****************************************************************
 *                     LISTS.C
 *
 *  PURPOSE: This file implements the functions to create, manage, and use
 *           linked lists.  You shouldn't have to modify this file.
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *
 *  FUNCTION LIST:
 *     createList
 *     createElement
 *     addElement
 *     addElementBefore
 *     deleteElement
 *     destroyList
 *     destroyElement
 *     removeElement
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LISTS_C

#include "lists.h"


/****** external global functions *****************/

/****** global functions **************************/

struct List *createList(char *);
struct Element *createElement(void *);
int addElement(struct List *, struct Element *);
int addElementBefore(struct List *, struct Element *, struct Element *);
struct Element *deleteElement(struct List *, struct Element *);
void destroyList(struct List *);
void destroyElement(struct Element *);
struct Element *removeElement(struct List *);

/****** static functions **************************/

/****** external global variables *****************/

/****** global variables **************************/

/****** static variables **************************/


/*******************************
 *                             *
 *  createList.c               *
 *                             *
 ******************************/
struct List *createList(char *name)
{
    struct List *aList;

    aList = (struct List *) malloc(sizeof(struct List));
    if (!aList) 
     {
	perror("createList:");
	exit(1);
     }
    strcpy(aList->name, name);
    aList->number = 0;
    aList->maxNumber = -1;	/* No limit set. */
    aList->head = aList->tail = NULL;

    return (aList);
}


/*******************************
 *                             *
 *  createElement.c            *
 *                             *
 ******************************/
struct Element *createElement(void *object)
{
    struct Element *anElement;
    anElement = (struct Element *) malloc(sizeof(struct Element));

    if (!anElement) 
     {
	perror("createElement:");
	return (NULL);
     }
    anElement->next = anElement->prev = NULL;
    anElement->Obj.object = object;

    return (anElement);
}


/*******************************
 *                             *
 *  addElement.c               *
 *                             *
 ******************************/
int addElement(struct List *aList, struct Element *anElement)
{

    if (aList->number == aList->maxNumber)
	return (LIST_ERROR_FULL);

    if (aList->number) 
     {	/* NOT EMPTY */
	aList->tail->next = anElement;
	anElement->prev = aList->tail;
	anElement->next = NULL;
	aList->tail = anElement;
     } 
    else 
     {
	aList->head = anElement;
	aList->tail = anElement;
	anElement->next = NULL;
	anElement->prev = NULL;
     }

    aList->number++;

    return (LIST_OK);
}


/*******************************
 *                             *
 *  addElementBefore.c         *
 *                             *
 ******************************/
int addElementBefore(struct List *aList, struct Element *beforeElement,
		 struct Element *anElement)
{

    if (aList->number == aList->maxNumber)
	return (LIST_ERROR_FULL);

    if (aList->number == 0) 
     {
	if (aList->name != NULL)
	    fprintf(stderr, "addElementBefore: list %s is empty\n", aList->name);
	exit(1);
     }

    if (beforeElement == aList->head) 
     {
	anElement->next = beforeElement;
	aList->head = anElement;
	anElement->prev = NULL;
	beforeElement->prev = anElement;
     }
    else 
     {
	anElement->prev = beforeElement->prev;
	anElement->prev->next = anElement;
	anElement->next = beforeElement;
	beforeElement->prev = anElement;
     }

    aList->number++;

    return (LIST_OK);
}


/*******************************
 *                             *
 *  deleteElement.c            *
 *                             *
 ******************************/
struct Element *deleteElement(struct List *aList, struct Element *anElement)
{
    if (aList->number == 0) 
     {
	if (aList->name != NULL)
	    fprintf(stderr, "deleteElement: list %s is empty\n", aList->name);
	exit(1);
     }

    if (aList->number == 1) 
     {
	aList->head = aList->tail = NULL;
     }
    else 
     {
	if (aList->head == anElement)
         {
	    aList->head = anElement->next;
	    aList->head->prev = NULL;
	 }
        else 
        if (aList->tail == anElement)
         {
	    aList->tail = anElement->prev;
	    aList->tail->next = NULL;
	 }
        else 
         {
	    anElement->prev->next = anElement->next;
	    anElement->next->prev = anElement->prev;
	 }
     }

    aList->number--;

    return (anElement);
}



/*******************************
 *                             *
 *  destroyList.c              *
 *                             *
 ******************************/
void destroyList(struct List *aList)
{
    void destroyElement(struct Element *);
    struct Element *anElement, *nextElement;

    if (aList->number) 
     {
	for (anElement = aList->head; anElement;) 
         {
	    nextElement = anElement->next;
	    deleteElement(aList, anElement);
	    destroyElement(anElement);
	    anElement = nextElement;
	 }
     }
    free(aList);
}


/*******************************
 *                             *
 *  destroyElement.c           *
 *                             *
 ******************************/
void destroyElement(struct Element *anElement)
{
    free(anElement);
}


/*******************************
 *                             *
 *  removeElement.c            *
 *                             *
 ******************************/
struct Element *removeElement(struct List *aList)
{
    struct Element *anElement = aList->head;

    if (aList->number) 
     {
	aList->head = aList->head->next;
	if (aList->head)
	    aList->head->prev = NULL;
	if (--aList->number == 0)
	    aList->tail = NULL;
     }
    else 
     {
	fprintf(stderr, "Tried to remove an element from an empty list.\n");
	if (aList->name != NULL)
	    fprintf(stderr, "List: %s\n", aList->name);
	exit(1);
     }

    return (anElement);
}
