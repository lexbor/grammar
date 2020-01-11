cmake_minimum_required(VERSION 2.8)

project(test_lexbor_grammar)

################
## Search and Includes
#########################
include_directories(".")

################
## Sources
#########################
file(GLOB_RECURSE TEST_LEXBOR_GRAMMAR_SOURCES "*.c")

################
## ARGS for tests
#########################
set(tokenizer_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/grammar/tokenizer")
set(parser_arg "${CMAKE_SOURCE_DIR}/test/files/lexbor/grammar/parser")

################
## Create tests
#########################
EXECUTABLE_LIST("lexbor_grammar_" "${TEST_LEXBOR_GRAMMAR_SOURCES}" ${TEST_UNIT_LIB_NAME} ${LEXBOR_LIB_NAME})
APPEND_TESTS("lexbor_grammar_" "${TEST_LEXBOR_GRAMMAR_SOURCES}")
