#ifndef AVL_TREE
#define AVL_TREE
#include <setjmp.h>
#include "../backend/types.hpp"
#include <inttypes.h>

struct Node;

class AvlTree
{
public:
    AvlTree(MachineDataTypes type, unsigned int maxDataSize);
    bool insertValue(char* key);
    bool find(char* key);
    int addRefrenceToRow(char* data, char* key);
private:
    int insert(char* key, Node** node);
    int rightRightBalance(Node* x);
    int leftLeftBalance(Node* x);
    int rightLeftBalance(Node* x);
    int leftRightBalance(Node* x);
private: 
    MachineDataTypes m_dataType;
    unsigned int m_maxDataSize;
    Node* m_root;
    jmp_buf m_jmp_buff;
    uint8_t m_triggerError;
};

#endif

