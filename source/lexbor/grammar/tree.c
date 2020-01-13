/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/grammar/tree.h"
#include "lexbor/grammar/node.h"

#include "lexbor/dom/interface.h"
#include "lexbor/dom/interfaces/element.h"


typedef struct lxb_grammar_tree_context {
    lxb_grammar_tree_group_t **groups;
    lxb_grammar_tree_entry_t **ends;
}
lxb_grammar_tree_context_t;


lxb_grammar_tree_t *
lxb_grammar_tree_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_grammar_tree_t));
}

lxb_status_t
lxb_grammar_tree_init(lxb_grammar_tree_t *tree,
                      lxb_grammar_document_t *document)
{
    lxb_status_t status;

    if (tree == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (document == NULL) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    tree->declarations = lexbor_bst_map_create();
    status = lexbor_bst_map_init(tree->declarations, 1024);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    tree->document = document;

    return LXB_STATUS_OK;
}

void
lxb_grammar_tree_clean(lxb_grammar_tree_t *tree)
{
    lexbor_bst_map_t *declarations = tree->declarations;

    lexbor_bst_map_clean(declarations);

    memset(tree, 0, sizeof(lxb_grammar_tree_t));

    tree->declarations = declarations;
}

lxb_grammar_tree_t *
lxb_grammar_tree_destroy(lxb_grammar_tree_t *tree, bool self_destroy)
{
    if (tree == NULL) {
        return NULL;
    }

    tree->declarations = lexbor_bst_map_destroy(tree->declarations, true);

    if (self_destroy) {
        return lexbor_free(tree);
    }

    return tree;
}

lxb_status_t
lxb_grammar_tree_declaration_reg(lxb_grammar_tree_t *tree,
                                 lxb_grammar_node_t *node)
{
    size_t len;
    const lxb_char_t *local_name;
    lexbor_bst_map_entry_t *entry;

    if (node->type != LXB_GRAMMAR_NODE_DECLARATION) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    local_name = lxb_dom_element_local_name(lxb_dom_interface_element(node),
                                            &len);

    entry = lexbor_bst_map_insert_not_exists(tree->declarations,
                                             &tree->declarations_root,
                                             local_name, len);
    if (entry == NULL) {
        return LXB_STATUS_ERROR;
    }

    entry->value = node;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_tree_make(lxb_grammar_tree_t *tree, lxb_grammar_node_t *group)
{
    lxb_grammar_node_t *node;

    if (group->type != LXB_GRAMMAR_NODE_DECLARATION
        && group->type != LXB_GRAMMAR_NODE_GROUP)
    {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    node = group->first_child;

    while (node != NULL) {


        node = node->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_tree_make_2(lxb_grammar_tree_t *tree, lxb_grammar_node_t *group)
{
    lxb_grammar_node_t *node;

    if (group->type != LXB_GRAMMAR_NODE_DECLARATION
        && group->type != LXB_GRAMMAR_NODE_GROUP)
    {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    node = group->first_child;

    while (node != NULL) {


        node = node->next;
    }

    return LXB_STATUS_OK;
}
