#include "parser.hpp"

using namespace std;


void consumeToken(ParsingState &state, TokenType type)
{
    Token token =  state.tokenizer.scan();
    if(token.type != type)
    {
        dprintf(2, "unexpected token in query\n");
        triggerParserError(state, 0, "unexpected token in query\n");
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
            freeNode(node);
        }
        statement = new AstNode{AstNodeType::ERROR};
        statement->data = new string( state.errorMessage );
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
    return nullptr;
}



AstNode *parseCreateTableStatement(ParsingState &state)
{
    consumeToken(state, TokenType::TABLE);
    AstNode* node = allocateNode(state);
    node->type = AstNodeType::CREATE_TABLE;

    AstNode* identifier = parseIdentifier(state);
    node->child.push_back( identifier );
    consumeToken(state, TokenType::L_PARENTHESES);
    Token token;
    do
    {
        AstNode* param = parseParameter(state);
        node->child.push_back(param);
        token = state.tokenizer.scan();
    } while (token.type == TokenType::COMMA );
    

    if(token.type != TokenType::R_PARENTHESES)
    {
        triggerParserError(state, 0, "Expected \')\' \n");
    }
    
    return node;
}

AstNode *parsePrimary(ParsingState &state)
{
    Token token = state.tokenizer.scan();
    AstNode* node = allocateNode(state);
    switch (token.type)
    {
    case TokenType::IDENTIFIER :
        node->type = AstNodeType::IDENTIFIER;
        node->data = token.data;
        break;
    case TokenType::CONSTANT :
        node->type = AstNodeType::CONSTANT;
        node->data = token.data;
        break;
    default:
        dprintf(2, "Unsupported primary\n");
        exit(-1);
        break;
    }
    return node;
}

AstNode *parseParameter(ParsingState &state)
{
    AstNode* name = parseIdentifier(state);
    AstNode* type = parseDataType(state);
    type->child.push_back(name);
    return type;
}

AstNode *parseDataType(ParsingState &state)
{
    Token token = state.tokenizer.scan();
    AstNode* type = nullptr;
    switch (token.type)
    {
    case TokenType::INT:
        type = allocateNode(state);
        type->type= AstNodeType::INT_32;
        break;
    case TokenType::CHAR:
        {
        type = allocateNode(state);
        type->type= AstNodeType::CHAR;
        consumeToken(state, TokenType::L_PARENTHESES);
        type->child.push_back( parseNumber(state) );
        consumeToken(state, TokenType::R_PARENTHESES);
        break;
        }
    default:
        triggerParserError(state, 0, "Unexpected data type\n");
        break;
    }
    return type;
}

AstNode *parseIdentifier(ParsingState &state)
{
    Token token = state.tokenizer.scan();
    if(token.type != TokenType::IDENTIFIER)
    {
        triggerParserError(state, 0, "Expected identifier\n");
    }

    AstNode* node = allocateNode(state);
    node->type = AstNodeType::IDENTIFIER;
    node->data = token.data;
    return node;
}

AstNode *parseNumber(ParsingState &state)
{
    Token token = state.tokenizer.scan();
    if(token.type != TokenType::CONSTANT)
    {
        triggerParserError(state, 0, "Expected constant\n");
    }

    AstNode* node = allocateNode(state);
    node->type = AstNodeType::CONSTANT;
    node->data = token.data;
    return node;
}

AstNode *allocateNode(ParsingState& state)
{
    AstNode* node = new AstNode();
    state.allNodes.push_back(node);
    return node;
}

void freeNode(AstNode *node)
{
    switch (node->type)
    {
    case AstNodeType::IDENTIFIER:
        delete ((string*)node->data);
        delete node;
        break;
    
    default:
        break;
    }

}

void triggerParserError(ParsingState &state, int value, const char* errorMessage)
{
    state.invalidQuery = true;
    state.errorMessage = errorMessage;
    longjmp(state.buff, value);
}
