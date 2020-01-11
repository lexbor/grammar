/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/fs.h>
#include <lexbor/core/array.h>

#include <unit/test.h>
#include <unit/kv.h>

#include <lexbor/grammar/tokenizer.h>
#include <lexbor/grammar/token.h>
#include <lexbor/grammar/parser.h>


typedef struct {
    unit_kv_t                  *kv;
    lexbor_str_t               str;
    lexbor_mraw_t              *mraw;
}
helper_t;


static lxb_status_t
parse(helper_t *helper, const char *dir_path);

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx);

static lxb_status_t
check(helper_t *helper, unit_kv_value_t *value);

static lxb_status_t
check_entry(helper_t *helper, unit_kv_value_t *entry,
            lxb_grammar_tokenizer_t *tkz, lxb_grammar_parser_t *parser);

static lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx);

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value);


int
main(int argc, const char * argv[])
{
    lxb_status_t status;
    helper_t helper = {0};
    const char *dir_path;

    if (argc != 2) {
        printf("Usage:\n\tgrammar_parser <directory path>\n");
        return EXIT_FAILURE;
    }

    dir_path = argv[1];

    TEST_INIT();

    helper.kv = unit_kv_create();
    status = unit_kv_init(helper.kv, 256);

    if (status != LXB_STATUS_OK) {
        goto done;
    }

    helper.mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(helper.mraw, 256);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    if(lexbor_str_init(&helper.str, helper.mraw, 1024) == NULL) {
        goto done;
    }

    status = parse(&helper, dir_path);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    TEST_RUN("lexbor/css/grammar/parser");
    TEST_RELEASE();

done:

    unit_kv_destroy(helper.kv, true);
    lexbor_mraw_destroy(helper.mraw, true);

    return EXIT_FAILURE;
}

static lxb_status_t
parse(helper_t *helper, const char *dir_path)
{
    lxb_status_t status;

    status = lexbor_fs_dir_read((const lxb_char_t *) dir_path,
                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, helper);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to read directory: %s", dir_path);
    }

    return status;
}


static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    lxb_status_t status;
    unit_kv_value_t *value;
    helper_t *helper;

    if (filename_len < 5 ||
        strncmp((const char *) &filename[ (filename_len - 4) ], ".ton", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    helper = ctx;

    TEST_PRINTLN("Parse file: %s", fullpath);

    unit_kv_clean(helper->kv);

    status = unit_kv_parse_file(helper->kv, (const lxb_char_t *) fullpath);
    if (status != LXB_STATUS_OK) {
        lexbor_str_t str = unit_kv_parse_error_as_string(helper->kv);

        TEST_PRINTLN("%s", str.data);

        unit_kv_string_destroy(helper->kv, &str, false);

        return EXIT_FAILURE;
    }

    value = unit_kv_value(helper->kv);
    if (value == NULL) {
        TEST_PRINTLN("Failed to get root value");
        return EXIT_FAILURE;
    }

    TEST_PRINTLN("Check file: %s", fullpath);

    status = check(helper, value);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    return LEXBOR_ACTION_OK;
}

static lxb_status_t
check(helper_t *helper, unit_kv_value_t *value)
{
    lxb_status_t status;
    unit_kv_array_t *entries;
    lxb_grammar_parser_t *parser;
    lxb_grammar_tokenizer_t *tkz;

    if (unit_kv_is_array(value) == false) {
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    entries = unit_kv_array(value);

    tkz = lxb_grammar_tokenizer_create();
    status = lxb_grammar_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    parser = lxb_grammar_parser_create();
    status = lxb_grammar_parser_init(parser);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    for (size_t i = 0; i < entries->length; i++) {
        if (unit_kv_is_hash(entries->list[i]) == false) {
            return print_error(helper, entries->list[i]);
        }

        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z, (i + 1));

        status = check_entry(helper, entries->list[i], tkz, parser);
        if (status != LXB_STATUS_OK) {
            goto failed;
        }

        lxb_grammar_tokenizer_clean(tkz);
    }

    lxb_grammar_tokenizer_destroy(tkz, true);
    lxb_grammar_parser_destroy(parser, true);

    return LXB_STATUS_OK;

failed:

    lxb_grammar_tokenizer_destroy(tkz, true);
    lxb_grammar_parser_destroy(parser, true);

    return LXB_STATUS_ERROR;
}

static lxb_status_t
check_entry(helper_t *helper, unit_kv_value_t *entry,
            lxb_grammar_tokenizer_t *tkz, lxb_grammar_parser_t *parser)
{
    lxb_grammar_document_t *document;
    lexbor_str_t *str_data, *str_result;
    unit_kv_value_t *data, *result;
    lxb_grammar_node_t *root;

    /* Validate */
    data = unit_kv_hash_value_nolen_c(entry, "data");
    if (data == NULL) {
        TEST_PRINTLN("Required parameter missing: data");

        return print_error(helper, entry);
    }

    if (unit_kv_is_string(data) == false) {
        TEST_PRINTLN("Parameter 'data' must be a STRING");

        return print_error(helper, data);
    }

    result = unit_kv_hash_value_nolen_c(entry, "result");
    if (result == NULL) {
        TEST_PRINTLN("Required parameter missing: result");

        return print_error(helper, entry);
    }

    if (unit_kv_is_string(result) == false) {
        TEST_PRINTLN("Parameter 'tokens' must be an STRING");

        return print_error(helper, result);
    }

    /* Parse */
    str_data = unit_kv_string(data);
    str_result = unit_kv_string(result);

    document = lxb_grammar_tokenizer_process(tkz, str_data->data,
                                             str_data->length);
    if (document == NULL) {
        return LXB_STATUS_ERROR;
    }

    root = lxb_grammar_parser_process(parser, document);
    if (root == NULL) {
        lxb_grammar_parser_print_last_error(parser);
        lxb_grammar_document_destroy(document);

        return LXB_STATUS_ERROR;
    }

    lexbor_str_clean(&helper->str);

    lxb_grammar_node_serialize_deep(root, serializer_callback, helper);

    lxb_grammar_document_destroy(document);

    if (str_result->length != helper->str.length
        || lexbor_str_data_ncmp(str_result->data, helper->str.data,
                                str_result->length) == false)
    {
        TEST_PRINTLN("Not match. \nHave:\n%s\nNeed:\n%s\n",
                     (const char *) helper->str.data,
                     (const char *) str_result->data);

        return print_error(helper, result);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    helper_t *helper = ctx;

    if(lexbor_str_append(&helper->str, helper->mraw, data, len) == NULL) {
        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value)
{
    lexbor_str_t str;

    str = unit_kv_value_position_as_string(helper->kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(helper->kv, &str, false);

    str = unit_kv_value_fragment_as_string(helper->kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(helper->kv, &str, false);

    return LXB_STATUS_ERROR;
}
