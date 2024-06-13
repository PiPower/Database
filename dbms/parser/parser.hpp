#ifndef PARSER
#define PARSER
#include <vector>
#include <string>
#include "tokenizer.hpp"
#include <setjmp.h>

enum class AstNodeType
{
    //misc
    IDENTIFIER,  CREATE_TABLE,
    PARAMS, CONSTANT, ERROR,
    // types
    NUMBER_32, CHAR

};


struct AstNode
{
    AstNodeType type;
    std::vector<AstNode*> child;
    void* data;
};


struct ParsingState
{
    const char* text;
    unsigned int ptr;
    Tokenizer tokenizer;
    bool invalidQuery;
    std::vector<AstNode*> allNodes;
    jmp_buf buff;
    const char* errorMessage;
};



void consumeToken(ParsingState& state, TokenType typ);
AstNode* allocateNode(ParsingState& state);
void freeNode(AstNode* node);
void triggerParserError(ParsingState& state, int value, const char* errorMessage = nullptr);

AstNode* parse(const char* text);
AstNode* parseStatement(ParsingState& state);
AstNode* parseCreateTableStatement(ParsingState& state);
AstNode* parsePrimary(ParsingState& state);
AstNode* parseParameter(ParsingState& state);
AstNode* parseDataType(ParsingState& state);
AstNode* parseIdentifier(ParsingState& state);
AstNode* parseNumber(ParsingState& state);

#endif