#ifndef JSW_AVLTREE_H
#define JSW_AVLTREE_H

#ifdef __cplusplus
#include <cstddef>

using std::size_t;

extern "C" {
#else
#include <stddef.h>
#endif

/* Opaque types */
typedef struct avltree avltree_t;
typedef struct avltrav avltrav_t;

/* User-defined item handling */
typedef int   (*cmp_f) ( const void *p1, const void *p2 );
typedef void *(*dup_f) ( void *p );
typedef void  (*rel_f) ( void *p );

/* AVL tree functions */
avltree_t *avlnew ( cmp_f cmp, dup_f dup, rel_f rel );
void           avldelete ( avltree_t *tree );
void          *avlfind ( avltree_t *tree, void *data );
int            avlinsert ( avltree_t *tree, void *data );
int            avlerase ( avltree_t *tree, void *data );
size_t         avlsize ( avltree_t *tree );

/* Traversal functions */
avltrav_t *avltnew ( void );
void           avltdelete ( avltrav_t *trav );
void          *avltfirst ( avltrav_t *trav, avltree_t *tree );
void          *avltlast ( avltrav_t *trav, avltree_t *tree );
void          *avltnext ( avltrav_t *trav );
void          *avltprev ( avltrav_t *trav );

#ifdef __cplusplus
}
#endif

#endif
