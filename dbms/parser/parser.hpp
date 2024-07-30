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
    INSERT, INSERT_ARGS, SELECT,
    SELECT_ARGS, STRING, INNER_JOIN_ON,
    TABLE_SPEC, TABLE_NAMES, WHERE,
    DELETE, PRIMARY_KEY,
    // table type specifier
    MAIN_TABLE, INNER_JOIN_TABLE,
    // types identifiers
    INT_32, CHAR,
    //math ops
    GREATER_EQUAL, GREATER, LESS_EQUAL, LESS, 
    EQUAL
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
    char* errorMessage;
};



void consumeToken(ParsingState& state, TokenType typ);
AstNode* allocateNode(ParsingState& state);
void freeNode(AstNode* node);
void triggerParserError(ParsingState& state, int value, const char* errorMessage = nullptr);

std::vector<AstNode*> parse(const char* text, char**  parserBuffer);
void freeParserBuffer(char* parserBuffer);
AstNode* parseStatement(ParsingState& state);
AstNode* parseCreateTableStatement(ParsingState& state);
AstNode* parseInsertStatement(ParsingState& state);
AstNode* parseSelectStatement(ParsingState& state);
AstNode* parseDeleteStatement(ParsingState& state);

AstNode* parsePrimary(ParsingState& state);
AstNode* parseParameter(ParsingState& state);
AstNode* parseDataType(ParsingState& state);
AstNode* parseIdentifier(ParsingState& state);
AstNode* parseNumber(ParsingState& state);
AstNode* parseTableSpecifier(ParsingState& state);
AstNode* parseArgument(ParsingState& state);
void parseExtensions(ParsingState& state, AstNode* root);
// epxressions
AstNode* parseExpression(ParsingState& state);
AstNode* parseOp(ParsingState& state);

#endif