#include "tokenizer.hpp"
#include <algorithm>


using namespace std;

Tokenizer::Tokenizer(const char *sourceCode)
:
m_source(sourceCode), m_offset(0)
{
    keywordMapInit();
}

Token Tokenizer::scan()
{
    while (true)
    {
    
        if(m_source[m_offset] == ' ' ||  m_source[m_offset] == '\t' ||  m_source[m_offset] == '\r' || m_source[m_offset] == '\n')
        {
            m_offset++;
            continue;
        }
        Token t = parseIdentifier();
        if(t.type != TokenType::NONE)
        {
            return t;
        }


    }
    

    return Token();
}

Token Tokenizer::peekToken()
{
    unsigned int m_offsetBuffer = m_offset;
    Token token = scan();
    m_offset = m_offsetBuffer;
    return token;
}

void Tokenizer::keywordMapInit()
{
    m_keywordMap["create"] = TokenType::CREATE;
    m_keywordMap["table"] = TokenType::TABLE;
}

Token Tokenizer::parseIdentifier()
{
     if(isDigit(m_source[m_offset]))
    {
        return Token{TokenType::NONE};
    }

    string currentString;
    while (isAlphaDigitFloor(m_source[m_offset]) )
    {
        currentString += m_source[m_offset];
        m_offset++;
    }

    transform(currentString.begin(), currentString.end(),  currentString.begin(), [](unsigned char c){ return std::tolower(c); });
    unordered_map<std::string, TokenType>::iterator iter = m_keywordMap.find(currentString);
    if(iter != m_keywordMap.end())
    {
        Token token;
        token.type = iter->second;
        return token;
    }

    Token token;
    token.type = TokenType::IDENTIFIER;
    return token;
}

bool Tokenizer::isDigit(const char &c)
{
    return '0' <= c  && c <= '9';
}

bool Tokenizer::isAlpha(const char &c)
{
    return ('A' <= c  && c <= 'Z') || ('a' <= c  && c <= 'z' );
}

bool Tokenizer::isAlphaDigitFloor(const char &c)
{
    return isDigit(c) || isAlpha(c) || c == '_';
}