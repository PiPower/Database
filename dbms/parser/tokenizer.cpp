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
        Token t = parsePunctuators();
        if(t.type != TokenType::NONE)
        {
            m_offset++;
            return t;
        }

        t = parseIdentifier();
        if(t.type != TokenType::NONE)
        {
            return t;
        }

        t = parseNumber();
        if(t.type != TokenType::NONE)
        {
            return t;
        }
    }
    
    return Token{TokenType::ERROR};
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
    m_keywordMap["int"] = TokenType::INT;
    m_keywordMap["char"] = TokenType::CHAR;
    m_keywordMap["from"] = TokenType::FROM;
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
    token.data = new string(currentString );
    return token;
}

Token Tokenizer::parsePunctuators()
{
    Token token;
    token.type = TokenType::NONE;
    switch (m_source[m_offset])
    {
    case ';':
        token.type = TokenType::SEMICOLON;
        break;
    case ',':
        token.type = TokenType::COMMA;
        break;
    case ':':
        token.type = TokenType::COLON;
        break;
    case '{':
        token.type = TokenType::L_BRACE;
        break;
    case '}':
        token.type = TokenType::R_BRACE;
        break;
    case '(':
        token.type = TokenType::L_PARENTHESES;
        break;
    case ')':
        token.type = TokenType::R_PARENTHESES;
        break;
    }
    return token;
}

Token Tokenizer::parseNumber()
{
    if( ! ('0' <= m_source[m_offset] && m_source[m_offset] <='9'))
    {
        return Token{TokenType::ERROR};
    }


    string value = "";
    while (isDigit(m_source[m_offset]))
    {
        value += m_source[m_offset++];
    }
    

    Token token;
    token.type = TokenType::CONSTANT;
    token.data = new string(value);
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