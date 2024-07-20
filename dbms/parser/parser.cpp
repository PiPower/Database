#include "parser.hpp"
#include <string.h>
#define ERROR_BUFFER_SIZE 500
using namespace std;

const char* errTable[]=
{
    "NONE", "ERROR", "IDENTIFIER", "END_OF_FILE",
    // keywords
    "CREATE", "TABLE", "FROM", "INT", "CHAR", "INSERT",
    "INTO", "VALUES", "SELECT", "WHERE",
    // separators
    "L_BRACKET",  "R_BRACKET", "L_PARENTHESES",  "R_PARENTHESES", 
    "L_BRACE", "R_BRACE",
    // miscallenous
    "COLON", "COMMA" , "SEMICOLON", "DOT",
    //types
    "CONSTANT", "STRING",
    //math ops
    "GREATER_EQUAL", "GREATER", "LESS_EQUAL", "LESS", 
    "EQUAL"
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
        return statements;
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

    AstNode* tables = allocateNode(state);
    tables->type = AstNodeType::TABLE_NAMES; 
    AstNode* tableName = parseIdentifier(state);
    //1st table in table names is always MAIN TABLE
    tableName->type = AstNodeType::MAIN_TABLE;

    tables->child.push_back(tableName);
    root->child.push_back(tables);

    parseSelectExtensions(state, root);
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
    case TokenType::STRING :
        node->type = AstNodeType::STRING;
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

AstNode *parseTableSpecifier(ParsingState &state)
{
    AstNode* tableSpec = nullptr;
    Token t1 = state.tokenizer.scan();
    Token t2 = state.tokenizer.scan();

    if(t2.type == TokenType::DOT && t1.type == TokenType::IDENTIFIER)
    {   
        tableSpec = allocateNode(state);
        tableSpec->type = AstNodeType::TABLE_SPEC;
        tableSpec->data = t1.data;
        tableSpec->child.push_back( parseIdentifier(state) );
    }
    else
    {
        state.tokenizer.putback(t1);
        state.tokenizer.putback(t2);
        tableSpec = parsePrimary(state);
    }
    return tableSpec;
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

void parseSelectExtensions(ParsingState &state, AstNode *root)
{
    while(true)
    {
        Token token = state.tokenizer.scan();
        switch (token.type)
        {
        case TokenType::WHERE:
        {
            AstNode* expression = parseExpression(state);
            AstNode* whereClause = allocateNode(state);
            whereClause->type = AstNodeType::WHERE;
            whereClause->child.push_back(expression);

            root->child.push_back(whereClause);
        }break;
        case TokenType::INNER:
        {
            consumeToken(state, TokenType::JOIN);
            AstNode* onTable = parseIdentifier(state);
            onTable->type = AstNodeType::INNER_JOIN_TABLE;
            root->child[1]->child.push_back(onTable);

            consumeToken(state, TokenType::ON);
            AstNode* expr = parseExpression(state);
            AstNode* onClause = allocateNode(state);
            onClause->type = AstNodeType::INNER_JOIN_ON;
            onClause->child.push_back(expr);
            root->child.push_back(onClause);
        }break;
        default:
            state.tokenizer.putback(token);
            return;
        }

    }
}

AstNode *parseExpression(ParsingState &state)
{
    AstNode* leftTerm = parseTableSpecifier(state);
    AstNode* op = parseOp(state);
    AstNode* rightTerm = parseTableSpecifier(state);
    op->child.push_back(leftTerm);
    op->child.push_back(rightTerm);
    return op;
}

AstNode *parseOp(ParsingState &state)
{
    Token t = state.tokenizer.scan();
    AstNode* op = allocateNode(state);
    switch (t.type)
    {
    case TokenType::GREATER :
        op->type = AstNodeType::GREATER;
        break;
    case TokenType::GREATER_EQUAL :
        op->type = AstNodeType::GREATER_EQUAL;
        break;
    case TokenType::LESS :
        op->type = AstNodeType::LESS;
        break;
    case TokenType::LESS_EQUAL :
        op->type = AstNodeType::LESS_EQUAL;
        break;
    case TokenType::EQUAL :
        op->type = AstNodeType::EQUAL;
        break;
    default:
        triggerParserError(state, 0, "unsupported operator");
        break;
    }
    return op;
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
    int errLen = strlen(errorMessage) + 1;
    if( errLen > ERROR_BUFFER_SIZE)
    {
        errLen = ERROR_BUFFER_SIZE;
    }
    memcpy( state.errorMessage,  errorMessage, errLen);
    longjmp(state.buff, value);
}
