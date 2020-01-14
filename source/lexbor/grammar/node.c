/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/grammar/node.h"
#include "lexbor/grammar/tokenizer.h"
#include "lexbor/grammar/parser.h"

#include "lexbor/core/conv.h"
#include "lexbor/html/serialize.h"


#define lxb_grammar_node_serialize_send(data, len, cb, ctx)                    \
    do {                                                                       \
        status = cb((const lxb_char_t *) data, len, ctx);                      \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_grammar_node_serialize_send_indent(count, cb, ctx)                 \
    do {                                                                       \
        for (size_t i = 0; i < count; i++) {                                   \
            lxb_grammar_node_serialize_send("  ", 2, cb, ctx);                 \
        }                                                                      \
    }                                                                          \
    while (0)


lxb_grammar_node_t *
lxb_grammar_node_create(lxb_grammar_parser_t *parser, lxb_grammar_token_t *token,
                        lxb_grammar_node_type_t type)
{
    lxb_grammar_node_t *node;
    lxb_grammar_document_t *document = parser->document;

    node = lexbor_mraw_calloc(document->dom_document.mraw,
                              sizeof(lxb_grammar_node_t));
    if (node == NULL) {
        return NULL;
    }

    node->type = type;
    node->token = token;
    node->document = parser->document;
    node->multiplier.start = -1;

    if (token == NULL) {
        return node;
    }

    switch (type) {
        case LXB_GRAMMAR_NODE_DECLARATION:
        case LXB_GRAMMAR_NODE_ELEMENT:
            node->u.node = token->u.node;
            break;

        case LXB_GRAMMAR_NODE_NUMBER:
            node->u.num = token->u.num;
            break;

        case LXB_GRAMMAR_NODE_STRING:
        case LXB_GRAMMAR_NODE_WHITESPACE:
        case LXB_GRAMMAR_NODE_DELIM:
        case LXB_GRAMMAR_NODE_UNQUOTED:
            node->u.str = token->u.str;
            break;

        default:
            break;
    }

    return node;
}

void
lxb_grammar_node_clean(lxb_grammar_node_t *node)
{
    lxb_grammar_document_t *document = node->document;

    memset(node, 0, sizeof(lxb_grammar_node_t));

    node->document = document;
}

lxb_grammar_node_t *
lxb_grammar_node_destroy(lxb_grammar_node_t *node)
{
    if (node == NULL) {
        return NULL;
    }

    return lexbor_mraw_free(node->document->dom_document.mraw, node);
}

void
lxb_grammar_node_insert_child(lxb_grammar_node_t *to, lxb_grammar_node_t *node)
{
    if (to->last_child != NULL) {
        to->last_child->next = node;
    }
    else {
        to->first_child = node;
    }

    node->parent = to;
    node->next = NULL;
    node->prev = to->last_child;

    to->last_child = node;
}

void
lxb_grammar_node_insert_before(lxb_grammar_node_t *to, lxb_grammar_node_t *node)
{
    if (to->prev != NULL) {
        to->prev->next = node;
    }
    else {
        if (to->parent != NULL) {
            to->parent->first_child = node;
        }
    }

    node->parent = to->parent;
    node->next = to;
    node->prev = to->prev;

    to->prev = node;
}

void
lxb_grammar_node_insert_after(lxb_grammar_node_t *to, lxb_grammar_node_t *node)
{
    if (to->next != NULL) {
        to->next->prev = node;
    }
    else {
        if (to->parent != NULL) {
            to->parent->last_child = node;
        }
    }

    node->parent = to->parent;
    node->next = to->next;
    node->prev = to;
    to->next = node;
}

void
lxb_grammar_node_remove(lxb_grammar_node_t *node)
{
    if (node->parent != NULL) {
        if (node->parent->first_child == node) {
            node->parent->first_child = node->next;
        }

        if (node->parent->last_child == node) {
            node->parent->last_child = node->prev;
        }
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    }

    node->parent = NULL;
    node->next = NULL;
    node->prev = NULL;
}

lxb_inline lxb_status_t
lxb_grammar_node_serialize_combinator(lxb_grammar_node_t *group,
                                      lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;

    switch (group->combinator) {
        case LXB_GRAMMAR_COMBINATOR_AND:
            lxb_grammar_node_serialize_send(" && ", 4, func, ctx);
            break;

        case LXB_GRAMMAR_COMBINATOR_OR:
            lxb_grammar_node_serialize_send(" || ", 4, func, ctx);
            break;

        case LXB_GRAMMAR_COMBINATOR_ONE_OF:
            lxb_grammar_node_serialize_send(" | ", 3, func, ctx);
            break;

        default:
            lxb_grammar_node_serialize_send(" ", 1, func, ctx);
            break;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_grammar_node_serialize_combinator_wo_ws(lxb_grammar_node_t *group,
                                     lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;

    switch (group->combinator) {
        case LXB_GRAMMAR_COMBINATOR_AND:
            lxb_grammar_node_serialize_send("&&", 2, func, ctx);
            break;

        case LXB_GRAMMAR_COMBINATOR_OR:
            lxb_grammar_node_serialize_send("||", 2, func, ctx);
            break;

        case LXB_GRAMMAR_COMBINATOR_ONE_OF:
            lxb_grammar_node_serialize_send("|", 1, func, ctx);
            break;

        default:
            lxb_grammar_node_serialize_send(" ", 1, func, ctx);
            break;
    }

    return LXB_STATUS_OK;
}

lxb_inline lxb_status_t
lxb_grammar_node_serialize_multiplier(lxb_grammar_node_t *node,
                                      lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t len;
    lxb_status_t status;
    lxb_grammar_period_t *multiplier = &node->multiplier;
    lxb_char_t buf[128];

    if (multiplier->start == -1) {
        return LXB_STATUS_OK;
    }

    if (multiplier->stop == -1) {
        if (multiplier->start == 0) {
            lxb_grammar_node_serialize_send("*", 1, func, ctx);
        }
        else if (multiplier->start == 1) {
            if (node->is_comma_separated) {
                lxb_grammar_node_serialize_send("#", 1, func, ctx);
            }
            else {
                lxb_grammar_node_serialize_send("+", 1, func, ctx);
            }
        }

        return LXB_STATUS_OK;
    }

    if (multiplier->start == 1 && multiplier->stop == 0) {
        lxb_grammar_node_serialize_send("!", 1, func, ctx);
        return LXB_STATUS_OK;
    }
    else if (multiplier->start == 0 && multiplier->stop == 1) {
        lxb_grammar_node_serialize_send("?", 1, func, ctx);
        return LXB_STATUS_OK;
    }

mod:

    if (node->is_comma_separated) {
        lxb_grammar_node_serialize_send("#", 1, func, ctx);
    }

    lxb_grammar_node_serialize_send("{", 1, func, ctx);

    len = lexbor_conv_float_to_data(multiplier->start, buf,
                                    (sizeof(buf) / sizeof(lxb_char_t)));
    lxb_grammar_node_serialize_send(buf, len, func, ctx);

    if (multiplier->start == multiplier->stop) {
        lxb_grammar_node_serialize_send("}", 1, func, ctx);

        return LXB_STATUS_OK;
    }

    lxb_grammar_node_serialize_send(",", 1, func, ctx);

    len = lexbor_conv_float_to_data(multiplier->stop, buf,
                                    (sizeof(buf) / sizeof(lxb_char_t)));

    lxb_grammar_node_serialize_send(buf, len, func, ctx);
    lxb_grammar_node_serialize_send("}", 1, func, ctx);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_node_serialize_deep(lxb_grammar_node_t *root,
                                lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;
    lxb_grammar_node_t *node;

    node = root;

    if (node->type == LXB_GRAMMAR_NODE_ROOT) {
        node = node->first_child;
    }

    while (node != NULL) {

        switch (node->type) {
            case LXB_GRAMMAR_NODE_GROUP:
                if (node->prev == NULL) {
                    lxb_grammar_node_serialize_send("[", 1, func, ctx);
                }
                else {
                    status = lxb_grammar_node_serialize_combinator(node->parent,
                                                                   func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }

                    lxb_grammar_node_serialize_send("[", 1, func, ctx);
                }

                if (node->first_child != NULL) {
                    node = node->first_child;
                    continue;
                }

                lxb_grammar_node_serialize_send("]", 1, func, ctx);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;

            case LXB_GRAMMAR_NODE_DECLARATION:
                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_node_serialize_send(" = ", 3, func, ctx);

                node = node->first_child;

                continue;

            default:
                if (node->prev) {
                    status = lxb_grammar_node_serialize_combinator(node->parent,
                                                                   func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;
        }

        while (node->next == NULL) {
            node = node->parent;

            if (node == NULL) {
                return LXB_STATUS_ERROR;
            }

            if (node->type == LXB_GRAMMAR_NODE_DECLARATION) {
                if (node->next != NULL) {
                    lxb_grammar_node_serialize_send("\n", 1, func, ctx);
                }
            }
            else if (node->type != LXB_GRAMMAR_NODE_ROOT) {
                lxb_grammar_node_serialize_send("]", 1, func, ctx);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            if (node == root) {
                return LXB_STATUS_OK;
            }
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_node_serialize(lxb_grammar_node_t *node,
                           lxb_grammar_serialize_cb_f func, void *ctx)
{
    size_t len;
    lxb_char_t buf[128];
    lxb_status_t status;

    switch (node->type) {
        case LXB_GRAMMAR_NODE_WHITESPACE:
        case LXB_GRAMMAR_NODE_DELIM:
        case LXB_GRAMMAR_NODE_UNQUOTED:
            return func(node->u.str.data, node->u.str.length, ctx);

        case LXB_GRAMMAR_NODE_DECLARATION:
        case LXB_GRAMMAR_NODE_ELEMENT:
            return lxb_html_serialize_cb(node->u.node, func, ctx);

        case LXB_GRAMMAR_NODE_STRING:
            status = func((lxb_char_t *) "\"", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            status = func(node->u.str.data, node->u.str.length, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            return func((lxb_char_t *) "\"", 1, ctx);

        case LXB_GRAMMAR_NODE_NUMBER:
            len = lexbor_conv_float_to_data(node->u.num, buf,
                                            (sizeof(buf) / sizeof(lxb_char_t)));
            return func(buf, len, ctx);

        case LXB_GRAMMAR_NODE_ROOT:
            return func((lxb_char_t *) "<ROOT>", 6, ctx);

        default:
            return func((lxb_char_t *) "UNDEFINED", 9, ctx);
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_grammar_node_serialize_ast(lxb_grammar_node_t *root,
                               lxb_grammar_serialize_cb_f func, void *ctx)
{
    lxb_status_t status;
    lxb_grammar_node_t *node;

    size_t indent = 0;

    node = root;

    if (node->type == LXB_GRAMMAR_NODE_ROOT) {
        node = node->first_child;
    }

    while (node != NULL) {

        switch (node->type) {
            case LXB_GRAMMAR_NODE_GROUP:
                lxb_grammar_node_serialize_send_indent(indent, func, ctx);
                lxb_grammar_node_serialize_send("<#GROUP>", 8, func, ctx);

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                if (node->combinator) {
                    lxb_grammar_node_serialize_send(", ", 2, func, ctx);

                    status = lxb_grammar_node_serialize_combinator_wo_ws(node,
                                                                     func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                lxb_grammar_node_serialize_send("\n", 1, func, ctx);

                indent++;

                if (node->first_child != NULL) {
                    node = node->first_child;
                    continue;
                }

                break;

            case LXB_GRAMMAR_NODE_DECLARATION:
                lxb_grammar_node_serialize_send_indent(indent, func, ctx);

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                if (node->combinator) {
                    lxb_grammar_node_serialize_send(", ", 2, func, ctx);

                    status = lxb_grammar_node_serialize_combinator_wo_ws(node,
                                                                     func, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                lxb_grammar_node_serialize_send("\n", 1, func, ctx);

                node = node->first_child;

                indent++;
                continue;

            default:
                lxb_grammar_node_serialize_send_indent(indent, func, ctx);

                status = lxb_grammar_node_serialize(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                status = lxb_grammar_node_serialize_multiplier(node, func, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                lxb_grammar_node_serialize_send("\n", 1, func, ctx);

                break;
        }

        while (node->next == NULL) {
            node = node->parent;

            indent--;

            if (node == NULL) {
                return LXB_STATUS_ERROR;
            }

            if (node == root) {
                return LXB_STATUS_OK;
            }
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}
