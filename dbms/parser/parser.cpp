#include "parser.hpp"

using namespace std;


void consumeToken(ParsingState &state, TokenType type)
{
    Token token =  state.tokenizer.scan();
    if(token.type != type)
    {
        dprintf(2, "invalid character in query\n");
        state.invalidQuery = true;
        longjmp(state.buff, 0);
    }
}

AstNode *parse(const char *text)
{
    ParsingState state{text, 0 , Tokenizer{text}, false };
    AstNode* statement;
    setjmp(state.buff);
    if(!state.invalidQuery)
    {
        statement = parseStatement(state);
    }
    else
    {
        for(AstNode* node : state.allNodes)
        {
            delete node;
        }
    }
    return statement;
}

AstNode* parseStatement(ParsingState& state)
{
    Token token =  state.tokenizer.scan();
    switch (token.type)
    {
    case TokenType::CREATE:
        return parseCreateTableStatement(state);
    }
}

AstNode *parseCreateTableStatement(ParsingState &state)
{
    consumeToken(state, TokenType::TABLE);
    return nullptr;
}

AstNode *allocateNode(ParsingState& state)
{
    AstNode* node = new AstNode();
    state.allNodes.push_back(node);
    return node;
}
