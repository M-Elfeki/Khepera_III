/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2006/10/27 08:53:20 $
 * $Revision: 1.2 $
 * ------------------------------------------------------------------------------*/
#include "kb_cel.h"

struct _fifo{
  int mutex;
  int length;
  kb_cel head, queue;
};

typedef struct _fifo * Fifo;

#define kb_fifo_fifoisEmpty(F) ((F)->length == 0)


Fifo kb_fifo_fifoEmpty();
int  kb_fifo_length(Fifo F);
void kb_fifo_enqueue(Fifo F,int kb_elt);
int kb_fifo_head(Fifo F);
void kb_fifo_remove(Fifo F);
int kb_fifo_dequeue(Fifo F);
void kb_fifo_printFifo(Fifo F);
Fifo kb_fifo_copy(Fifo F);
