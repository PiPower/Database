#ifndef TOKENIZER
#define TOKENIZER
#include <vector>
#include <unordered_map>
#include <string>
enum class TokenType
{
    NONE, IDENTIFIER, END_OF_FILE,
    // keywords
    CREATE, TABLE
};


struct Token
{
    TokenType type;
};


class Tokenizer
{

public:
    Tokenizer(const char* sourceCode);
    Token scan();
    Token peekToken();
    bool match(TokenType type);
    void consume(TokenType type);
private:
    void keywordMapInit();
    Token parseIdentifier();

    bool isDigit(const char& c);
    bool isAlpha(const char& c);
    bool isAlphaDigitFloor(const char& c);

private:
    unsigned int m_offset;
    const char* m_source;
    std::unordered_map<std::string, TokenType> m_keywordMap;
};



#endif