/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_TREE_H
#define LEXBOR_GRAMMAR_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"

#include "lexbor/html/interfaces/document.h"

#include "lexbor/core/bst_map.h"


typedef lxb_status_t
(*lxb_grammar_tree_state_f)(lxb_grammar_tree_t *tree,
                            void *s);

struct lxb_grammar_tree_group {
    lxb_grammar_tree_group_t *prev;

    lxb_grammar_tree_entry_t *first_entry;
    lxb_grammar_tree_entry_t *last_entry;
};

struct lxb_grammar_tree_entry {
    lxb_grammar_tree_group_t *to;
    lxb_grammar_tree_group_t *from;

    lxb_grammar_node_t       *node;
};

struct lxb_grammar_tree {
    lxb_grammar_tree_state_f *state;
    lxb_grammar_tree_group_t *root;
    lxb_grammar_tree_group_t *current;

    lxb_grammar_document_t   *document;

    lexbor_bst_map_t         *declarations;
    lexbor_bst_entry_t       *declarations_root;
};


lxb_grammar_tree_t *
lxb_grammar_tree_create(void);

lxb_status_t
lxb_grammar_tree_init(lxb_grammar_tree_t *tree,
                      lxb_grammar_document_t *document);

void
lxb_grammar_tree_clean(lxb_grammar_tree_t *tree);

lxb_grammar_tree_t *
lxb_grammar_tree_destroy(lxb_grammar_tree_t *tree, bool self_destroy);


lxb_status_t
lxb_grammar_tree_declaration_reg(lxb_grammar_tree_t *tree,
                                 lxb_grammar_node_t *node);

/*
 * Inline functions
 */
lxb_inline lxb_grammar_tree_group_t *
lxb_grammar_tree_group_create(lxb_grammar_tree_t *tree)
{
    return lexbor_mraw_calloc(tree->document->dom_document.mraw,
                              sizeof(lxb_grammar_tree_group_t));
}

lxb_inline lxb_grammar_tree_entry_t *
lxb_grammar_tree_entry_create(lxb_grammar_tree_t *tree)
{
    return lexbor_mraw_calloc(tree->document->dom_document.mraw,
                              sizeof(lxb_grammar_tree_entry_t));
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_TREE_H */
