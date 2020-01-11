
#include "lexbor/grammar/tokenizer.h"
#include "lexbor/grammar/parser.h"


static lxb_status_t
serializer_callback(const lxb_char_t *data, size_t length, void *ctx)
{
    printf("%.*s", (int) length, data);

    return LXB_STATUS_OK;
}

int
main(int argc, const char * argv[])
{
    lxb_status_t status;
    lxb_grammar_parser_t *parser;
    lxb_grammar_document_t *doc;
    lxb_grammar_tokenizer_t *tkz;
    lxb_grammar_node_t *root;

    lxb_char_t grammar[] =
    "<a> = <num>* [<a> <x> | <y> | <c> <z> && <b> || <m> <h> | <z>]{1,2} <str>*";

    printf("In: %s\n", (char *) grammar);

    /* Create tokenizer and process data. */
    tkz = lxb_grammar_tokenizer_create();
    status = lxb_grammar_tokenizer_init(tkz);
    if (tkz == NULL) {
        exit(EXIT_FAILURE);
    }

    doc = lxb_grammar_tokenizer_process(tkz, grammar, sizeof(grammar) - 1);
    if (doc == NULL) {
        exit(EXIT_FAILURE);
    }

    lxb_grammar_tokenizer_destroy(tkz, true);

    /* Parse tokens. */
    parser = lxb_grammar_parser_create();
    status = lxb_grammar_parser_init(parser);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    root = lxb_grammar_parser_process(parser, doc);
    if (root == NULL) {
        lxb_grammar_parser_print_last_error(parser);
        lxb_grammar_document_destroy(doc);

        exit(EXIT_FAILURE);
    }

    lxb_grammar_parser_destroy(parser, true);

    printf("Out: ");

    status = lxb_grammar_node_serialize_deep(root, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    printf("\n");

    lxb_grammar_document_destroy(doc);
}
