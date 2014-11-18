/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2006/10/27 08:53:20 $
 * $Revision: 1.2 $
 * ------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "kb_fifo.h"

/*! 
 * \file   kb_fifo.c library for Fifo (first in first out).             
 *
 * \brief 
 *         This API is a simple Fifo 
 *
 * \author   Baptiste Berger (K-Team SA)
 *
 * \note     Copyright (C) 2005 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */


/*--------------------------------------------------------------------*/
/*! 
 * This function return a Empty Fifo
 * 
 * \return        The Empty Fifo	
 */
Fifo kb_fifo_fifoEmpty() {
   Fifo F;

   F = (Fifo)malloc(sizeof(struct _fifo));
   if (F == NULL) perror("Problem fifo is empty \n\r");
   F->length = 0;
   F->mutex = 0;
   F->head = F->queue = NULL;
   return(F);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the length of a Fifo queue.
 * 
 * \param F       Fifo Queue
 * \return        the size of Fifo Queue		
 */
int kb_fifo_length(Fifo F) {
   if (F == NULL) perror("Problem fifo length, F is null \n\r");
   return(F->length);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function enqueue a element in the fifo.
 * 
 * \param F       Fifo Queue
 * \param elt     is the pointer of elt 		
 */
void kb_fifo_enqueue(Fifo F, int elt) {
   kb_cel cel;

   if (F == NULL) perror("enqueue problem, F is Null \n\r");
   cel = (kb_cel)malloc(sizeof(struct _cel));
   if (cel == NULL) perror("enqueue problem, cel is Null");
   cel->elt = elt;
   cel->next = NULL;
   while((F->mutex) != 0);
   (F->mutex) = 1;
   if (kb_fifo_length(F) == 0)
      F->head = F->queue = cel;
   else {
      F->queue->next = cel;
      F->queue = cel;
   }
   ++(F->length);
   (F->mutex) = 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return, but not dequeue the head element in the fifo.
 * 
 * \param F       Fifo Queue
 * \return	  the head element	
 */
int kb_fifo_head(Fifo F) {
   if (F == NULL || kb_fifo_length(F) == 0) perror("head problem, F is Null or size = 0 \n\r");
   return(F->head->elt);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function remove the head element in the fifo.
 * 
 * \param F       Fifo Queue		
 */
void kb_fifo_remove(Fifo F) {
   kb_cel cel;

   if (F == NULL || kb_fifo_length(F) == 0) perror("dequeue problem, F is null or size = 0 \n\r");
   
   while((F->mutex) != 0);
   (F->mutex) = 1;
   cel = F->head;
   if (kb_fifo_length(F) == 1)
      F->head = F->queue = NULL;
   else
      F->head = F->head->next;
   --(F->length);
   (F->mutex) = 0;
   free(cel);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function dequeue and return the head element in the fifo.
 * 
 * \param F       Fifo Queue
 * \return        the head element
 */
int kb_fifo_dequeue(Fifo F) { 
   kb_cel cel;
   int elt;

   if (F == NULL || kb_fifo_length(F) == 0) perror("run problem, F is null or size = 0 \n\r");
   
   while((F->mutex) != 0);
   (F->mutex) = 1;
   cel = F->head;
   elt = cel->elt;
   if (kb_fifo_length(F) == 1)
      F->head = F->queue = NULL;
   else
      F->head = F->head->next;
   free(cel);
   --(F->length);
   (F->mutex) = 0;
   return(elt);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function copy the fifo and return the clone.
 * 
 * \param F       Fifo Queue
 * \return        the Fifo Clone		
 */
Fifo kb_fifo_copy(Fifo F) {
   Fifo G, H;
   kb_cel cel;

   if (F == NULL) perror("copy problem, F is null \n\r");
   H = kb_fifo_fifoEmpty();
   cel = F->head;
   while (cel != NULL) {
      kb_fifo_enqueue(H, cel->elt);
      cel = cel->next;
   }
   G = kb_fifo_fifoEmpty();
   cel = H->head;
   while (cel != NULL) {
      kb_fifo_enqueue(G, cel->elt);
      cel = cel->next;
   }
   return(G);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function print a Fifo (but this fonction is only with int element)
 * 
 * \param F       Fifo Queue		
 */
void kb_fifo_printFifo(Fifo F) {
   kb_cel cel;

   if (F == NULL) perror("print problem, F is null \n\r");
   cel = F->head;
   while (cel != NULL) {
     printf("%d ", cel->elt);
     cel = cel->next;
   }
   printf("\n\r");
}
