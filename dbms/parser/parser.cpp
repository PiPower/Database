#include "parser.hpp"
using namespace std;
struct ParsingState
{
    const char* text;
    unsigned int ptr;

};




AstNode* parse(const char* text)
{
    ParsingState state;
    state.text = text;
    state.ptr = 0;
    
    return nullptr;
}