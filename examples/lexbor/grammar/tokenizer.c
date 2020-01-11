
#include "lexbor/grammar/tokenizer.h"


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
    lexbor_array_t *tokens;
    lxb_grammar_token_t *token;
    lxb_grammar_document_t *doc;
    lxb_grammar_tokenizer_t *tkz;

    size_t len;
    const lxb_char_t *name;

    lxb_char_t grammar[] = "<a> = <num>* [<a> <x> | <b> <y> | <c> <z>] <str>*";

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

    /* Serialize tokens. */
    tokens = lxb_grammar_tokenizer_tokens(doc);

    for (size_t i = 0; i < tokens->length; i++) {
        token = tokens->list[i];

        name = lxb_grammar_token_name(token, &len);
        serializer_callback(name, len, NULL);
        serializer_callback((lxb_char_t *) ": ", 2, NULL);

        status = lxb_grammar_token_serialize(token, serializer_callback, NULL);
        if (status != LXB_STATUS_OK) {
            return EXIT_FAILURE;
        }

        if ((i + 1) != tokens->length)  {
            serializer_callback((lxb_char_t *) "\n", 1, NULL);
        }
    }

    printf("\n");

    lxb_grammar_document_destroy(doc);
}
