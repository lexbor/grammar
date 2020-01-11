cmake_minimum_required(VERSION 2.8)

project("examples_lexbor_grammar")

################
## Sources
#########################
file(GLOB_RECURSE EXAMPLES_LEXBOR_GRAMMAR_SOURCES "*.c")

################
## Create executable
#########################
EXECUTABLE_LIST("" "${EXAMPLES_LEXBOR_GRAMMAR_SOURCES}" ${LEXBOR_LIB_NAME})
