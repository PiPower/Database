#ifndef PARSER
#define PARSER
#include <vector>
#include <string>

struct AstNode
{
    std::vector<AstNode*> child;
};


AstNode* parse(const char* text);


#endif