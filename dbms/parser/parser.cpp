#include "parser.hpp"
#include <string.h>
#define ERROR_BUFFER_SIZE 500
using namespace std;

const char* errTable[]=
{
    "NONE", "ERROR", "IDENTIFIER", "END_OF_FILE",
    // keywords
    "CREATE", "TABLE", "FROM", "INT", "CHAR", "INSERT",
    "INTO", "VALUES", "SELECT",
    // separators
    "L_BRACKET",  "R_BRACKET", "L_PARENTHESES",  "R_PARENTHESES", 
    "L_BRACE", "R_BRACE",
    // miscallenous
    "COLON", "COMMA" , "SEMICOLON", "DOT",
    //types
    "CONSTANT", "STRING"
};



void consumeToken(ParsingState &state, TokenType type)
{
    Token token =  state.tokenizer.scan();
    if(token.type != type)
    {
        snprintf(state.errorMessage, ERROR_BUFFER_SIZE,"Expected token is %s but given is %s", errTable[(uint16_t)type], errTable[ (uint16_t)token.type]);
        triggerParserError(state, 0, state.errorMessage);
    }
}

std::vector<AstNode*> parse(const char *text)
{
    ParsingState state{text, 0 , Tokenizer{text}, false };
    state.errorMessage = new char[ERROR_BUFFER_SIZE];
    vector<AstNode*> statements;
    setjmp(state.buff);
    if(state.invalidQuery)
    {
        for(AstNode* node : state.allNodes)
        {
            freeNode(node);
        }
        statements.clear();
        statements.push_back( new AstNode{AstNodeType::ERROR} );
        statements[0]->data = new string( state.errorMessage );
    }
    while (state.tokenizer.peekToken().type != TokenType::END_OF_FILE)
    {
        statements.push_back( parseStatement(state) );
    }
    return statements;
}

AstNode* parseStatement(ParsingState& state)
{
    Token token =  state.tokenizer.scan();
    AstNode* statement = nullptr;
    switch (token.type)
    {
    case TokenType::CREATE:
        statement = parseCreateTableStatement(state);
        break;
    case TokenType::INSERT:
        statement = parseInsertStatement(state);
        break;
    case TokenType::SELECT:
        statement = parseSelectStatement(state);
        break;
    }

    consumeToken(state, TokenType::SEMICOLON);
    return statement;
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
    
    state.tokenizer.putback(token);
    consumeToken(state, TokenType::R_PARENTHESES);
    
    return node;
}

AstNode *parseInsertStatement(ParsingState &state)
{
    AstNode* root = allocateNode(state);
    root->type = AstNodeType::INSERT;

    consumeToken(state, TokenType::INTO);
    AstNode* tableName = parseIdentifier(state);
    root->child.push_back(tableName);
    root->child.push_back(nullptr);
    consumeToken(state, TokenType::VALUES);
    Token token;
    do
    {   
        AstNode* args = allocateNode(state);
        args->type = AstNodeType::INSERT_ARGS;
        root->child.push_back(args);
        consumeToken(state, TokenType::L_PARENTHESES);

        do
        {
            args->child.push_back( parseArgument(state) );
            token = state.tokenizer.scan();
        } while (token.type == TokenType::COMMA );
        state.tokenizer.putback(token);
        consumeToken(state, TokenType::R_PARENTHESES);

        token = state.tokenizer.scan();
    } while (token.type == TokenType::COMMA);

    state.tokenizer.putback(token);
    return root;
}

AstNode *parseSelectStatement(ParsingState &state)
{
    AstNode* root = allocateNode(state);
    root->type = AstNodeType::SELECT;

    AstNode* node = allocateNode(state);
    node->type = AstNodeType::SELECT_ARGS;

    root->child.push_back(node);
    Token token;
    do
    {
        AstNode* arg = parseIdentifier(state);
        node->child.push_back(arg);

        token = state.tokenizer.scan();
    } while (token.type == TokenType::COMMA);
    state.tokenizer.putback(token);

    consumeToken(state, TokenType::FROM);

    AstNode* tableName = parseIdentifier(state);
    root->child.push_back(tableName);
    return root;
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
        triggerParserError(state, 0, "expecter primary or constant");
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

AstNode *parseArgument(ParsingState &state)
{
    Token token = state.tokenizer.scan();
    if(token.type != TokenType::STRING && token.type != TokenType::CONSTANT)
    {
        triggerParserError(state, 0, "Unsupported type in argument");
    }
    AstNode* argument = allocateNode(state);

    switch (token.type)
    {
    case TokenType::STRING:
        argument->type = AstNodeType::STRING;
        argument->data = token.data;
        break;
    case TokenType::CONSTANT:
        argument->type = AstNodeType::CONSTANT;
        argument->data = token.data;
        break;
    default:
        triggerParserError(state, 0, "Unsupported type in argument");
        break;
    }

    return argument;
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
    int errLen = strlen(errorMessage  + 1);
    if( errLen > ERROR_BUFFER_SIZE)
    {
        errLen = ERROR_BUFFER_SIZE;
    }
    memcpy( state.errorMessage,  errorMessage, errLen);
    longjmp(state.buff, value);
}
