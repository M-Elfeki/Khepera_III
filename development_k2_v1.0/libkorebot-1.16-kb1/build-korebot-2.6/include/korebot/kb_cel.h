/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2006/10/27 08:53:20 $
 * $Revision: 1.2 $
 * ----------------------------------------------------------------------------*/

struct _cel {
   int elt;
   struct _cel *next;
};
typedef struct _cel *kb_cel;
