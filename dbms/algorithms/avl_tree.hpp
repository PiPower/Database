#ifndef AVL_TREE
#define AVL_TREE
#include <setjmp.h>
#include <vector>
#include "../backend/types.hpp"
#include <inttypes.h>
#include <string>

struct Node
{
    int64_t m_key_int;

    char m_balanceFactor;

    std::vector<std::string> refrences;
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
    void clear();
private:
    int insert(char* key, Node** node);
    Node* rightRightBalance(Node* x);
    Node* leftLeftBalance(Node* x);
    Node* rightLeftBalance(Node* x);
    Node* leftRightBalance(Node* x);
    void rebalanceNoChildNode(Node* y, bool childIsLeftNode);
    void rebalanceRecursive(Node* y, Node* oldY);
    void rebalanceAfterDeletion(Node* y, Node* child);
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

