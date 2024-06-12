#ifndef PARSER
#define PARSER
#include <vector>
#include <string>
#include "tokenizer.hpp"
#include <setjmp.h>

struct AstNode
{
    std::vector<AstNode*> child;
};


struct ParsingState
{
    const char* text;
    unsigned int ptr;
    Tokenizer tokenizer;
    bool invalidQuery;
    std::vector<AstNode*> allNodes;
    jmp_buf buff;
};



void consumeToken(ParsingState& state, TokenType typ);

AstNode* parse(const char* text);
AstNode* parseStatement(ParsingState& state);
AstNode* parseCreateTableStatement(ParsingState& state);
AstNode* allocateNode(ParsingState& state);
#endif