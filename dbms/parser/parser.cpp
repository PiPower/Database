#include "parser.hpp"
using namespace std;

AstNode* parse(const char* text)
{
    ParsingState state{text, 0 , Tokenizer{text} };

    return parseStatement(state);
}

AstNode* parseStatement(ParsingState& state)
{
    Token token =  state.tokenizer.scan();
    
}