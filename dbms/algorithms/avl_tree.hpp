#ifndef AVL_TREE
#define AVL_TREE
#include <setjmp.h>
#include "../backend/types.hpp"
#include <inttypes.h>

struct Node
{
    long long int m_key_int;

    char m_balanceFactor;

    char** m_rowRefrences;
    Node* m_leftChild;
    Node* m_rightChild;
    Node* m_parent;
};


class AvlTree
{
public:
    AvlTree(MachineDataTypes type, unsigned int maxDataSize);
    bool insertValue(char* key);
    bool find(char* key);
    bool removeValue(char* key);
    int addRefrenceToRow(char* data, char* key);
    void clear();
private:
    int insert(char* key, Node** node);
    int rightRightBalance(Node* x);
    int leftLeftBalance(Node* x);
    int rightLeftBalance(Node* x);
    int leftRightBalance(Node* x);

#ifdef TEST_AVL_TREE
public:
#else
private: 
#endif
    MachineDataTypes m_dataType;
    unsigned int m_maxDataSize;
    Node* m_root;
    jmp_buf m_jmp_buff;
    uint8_t m_triggerError;
};

#endif

