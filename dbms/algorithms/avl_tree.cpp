#include "avl_tree.hpp"
#include <math.h>
#include <utility>

using namespace std;

struct Node
{
    long long int m_key_int;

    char balanceFactor;

    char** m_rowRefrences;
    Node* m_leftChild;
    Node* m_rightChild;
    Node* m_parent;
};

AvlTree::AvlTree(MachineDataTypes type, unsigned int maxDataSize)
: m_dataType(type), m_maxDataSize(maxDataSize), m_root(nullptr)
{
}

//inserts value into tree
//return true if value has been inserted
bool AvlTree::insertValue(char *key)
{
    int n = insert(key, &m_root);
    if(n)
    {
        return false;
    }
    return true;
}

//return true if value is inside tree
bool AvlTree::find(char *key)
{
    return false;
}

//finds node whose value is equal to key, then adds
//refrence to the record and return 0
//if there is no value equal to key function return -1 and
//that not add refrence to record
int AvlTree::addRefrenceToRow(char *data, char *key)
{
    return 0;
}

//return 1 if values is found, otherwise return 0 
int AvlTree::insert(char *key, Node** node)
{
    Node* parent = nullptr;
    while(true)
    {
        if(!(*node))
        {
            *node = new Node();
            (*node)->m_rightChild = nullptr;
            (*node)->m_leftChild = nullptr;
            (*node)->m_key_int = *(int*)key;
            (*node)->m_rowRefrences = nullptr;
            (*node)->m_parent = parent;
            (*node)->balanceFactor = 0;
            break;
        }

        int comp = (*node)->m_key_int - *(int*)key;
        if(comp == 0 )
        {
            return 1;
        }

        parent = *node;
        if(comp > 0)
        {
            node = &(*node)->m_leftChild;
        }
        else
        {
            node = &(*node)->m_rightChild;
        }
    }

    // rebalancing 
    Node* child = *node;
    while (parent != nullptr)
    {
        if(child == parent->m_leftChild)
        {
            parent->balanceFactor--;
            
            // left heavy, needs rebalance
            if(parent->balanceFactor == -2)
            {
                // left left rotation
                if(child->balanceFactor <= 0 )
                {
                    leftLeftBalance(parent);
                    break;
                }
                // left right rotation
                else
                {
                   break;
                }   
            }
        }
        else
        {
            parent->balanceFactor++;
            // right heavy, needs rebalance
            if(parent->balanceFactor == 2)
            {
                // right left rotation
                if(child->balanceFactor < 0 )
                {
                    break;
                }
                // right right rotation
                else
                {
                    rightRightBalance(parent);
                    break;
                }
            }
        }


        child = parent;
        parent = child->m_parent;
    }
    
    return 0;
}

int AvlTree::rightRightBalance(Node* x)
{
    Node* z = x->m_rightChild;
    Node* holder =z->m_leftChild;
    //first swap
   z->m_leftChild = x;
   z->m_parent = x->m_parent;
    if(x->m_parent)
    {
        //update parent pointer
        Node* parent = x->m_parent;
        if(parent->m_leftChild == x) {parent->m_leftChild =z;} 
        else{parent->m_rightChild =z;}
    }
    else
    {
        // no parent pointer means we have root
        m_root =z;
    }
    x->m_parent =z;
    //second swap
    x->m_rightChild = holder;
    if(holder)
    {
        holder->m_parent = x;
    }

    z->balanceFactor = 0;
    x->balanceFactor = 0;
  
    return 0;
}

int AvlTree::leftLeftBalance(Node *x)
{
    Node* z = x->m_leftChild;
    Node* holder = z->m_rightChild;
    // first swap
    z->m_rightChild = x;
    z->m_parent = x->m_parent;
    if(x->m_parent)
    {
        //update parent pointer
        Node* parent = x->m_parent;
        if(parent->m_leftChild == x) {parent->m_leftChild =z;} 
        else{parent->m_rightChild =z;}
    }
    else
    {
        // no parent pointer means we have root
        m_root =z;
    }
    x->m_parent =z;
    // second swap
    x->m_leftChild = holder;
    if(holder)
    {
        holder->m_parent = x;
    }

   z->balanceFactor = 0;
    x->balanceFactor = 0;
    return 0;
}
