#ifndef TOKENIZER
#define TOKENIZER
#include <vector>
#include <unordered_map>
#include <string>
enum class TokenType
{
    NONE, ERROR, IDENTIFIER, END_OF_FILE,
    // keywords
    CREATE, TABLE, FROM, INT, CHAR,
    // separators
    L_BRACKET,  R_BRACKET, L_PARENTHESES,  R_PARENTHESES, 
    L_BRACE, R_BRACE,
    // miscallenous
     COLON, COMMA , SEMICOLON, DOT,
    //types
    CONSTANT
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

private:
    void keywordMapInit();
    Token parseIdentifier();
    Token parsePunctuators();
    Token parseNumber();

    bool isDigit(const char& c);
    bool isAlpha(const char& c);
    bool isAlphaDigitFloor(const char& c);

private:
    unsigned int m_offset;
    const char* m_source;
    std::unordered_map<std::string, TokenType> m_keywordMap;
};



#endif