#ifndef TOKENIZER
#define TOKENIZER
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
enum class TokenType
{
    NONE, ERROR, IDENTIFIER, END_OF_FILE,
    // keywords
    CREATE, TABLE, FROM, INT, CHAR, INSERT,
    INTO, VALUES, SELECT, WHERE, INNER, JOIN,
    ON,
    // separators
    L_BRACKET,  R_BRACKET, L_PARENTHESES,  R_PARENTHESES, 
    L_BRACE, R_BRACE,
    // miscallenous
    COLON, COMMA , SEMICOLON, DOT,
    //types
    CONSTANT, STRING,
    // math ops
    GREATER_EQUAL, GREATER, LESS_EQUAL, LESS, 
    EQUAL
};


struct Token
{
    TokenType type;
    void *data;
};


class Tokenizer
{

public:
    Tokenizer(const char* sourceCode);
    Token scan();
    Token peekToken();
    void putback(Token token);
private:
    void keywordMapInit();
    Token parseIdentifier();
    Token parsePunctuators();
    Token parseNumber();
    Token parseString();

    bool isDigit(const char& c);
    bool isAlpha(const char& c);
    bool isAlphaDigitFloor(const char& c);

private:
    unsigned int m_offset;
    const char* m_source;
    std::unordered_map<std::string, TokenType> m_keywordMap;\
    std::queue<Token> putbackQueue;
};



#endif