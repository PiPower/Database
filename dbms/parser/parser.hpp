#ifndef PARSER
#define PARSER
#include <vector>
#include <string>
#include "tokenizer.hpp"

struct ParsingState
{
    const char* text;
    unsigned int ptr;
    Tokenizer tokenizer;
};

struct AstNode
{
    std::vector<AstNode*> child;
};


AstNode* parse(const char* text);
AstNode* parseStatement(ParsingState& state);

#endif