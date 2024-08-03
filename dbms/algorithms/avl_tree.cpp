#include "avl_tree.hpp"
#include <math.h>
#include <utility>
#include <queue>
using namespace std;

AvlTree::AvlTree(MachineDataTypes type, unsigned int maxDataSize)
: m_dataType(type), m_maxDataSize(maxDataSize), m_root(nullptr)
{
}

//inserts value into tree
//return true if value has been inserted
//if specified value exists inside tree 
//dont insert it again and return false
bool AvlTree::insertValue(char *key)
{
    int n = insert(key, &m_root);
    if(!n)
    {
        return false;
    }
    return true;
}

//return true if value is inside tree
bool AvlTree::find(char *key)
{
    Node* currentNode = m_root;
    while(currentNode)
    {
        int comp = currentNode->m_key_int - *(int*)key;
        if(comp == 0 )
        {
            return true;
        }

        if(comp > 0)
        {
            currentNode = currentNode->m_leftChild;
        }
        else
        {
            currentNode = currentNode->m_rightChild;
        }
    }

    return false;
}


bool AvlTree::removeValue(char *key)
{             
    Node* currentNode = m_root;
    while(currentNode)
    {
        int comp = currentNode->m_key_int - *(int*)key;
        if(comp == 0 )
        {   
            break;
        }

        if(comp > 0)
        {
            currentNode = currentNode->m_leftChild;
        }
        else
        {
            currentNode = currentNode->m_rightChild;
        }
    }

    if(!currentNode)
    {
        return false;
    }
    //remove found node
    Node* parentNode = currentNode->m_parent;
    Node* retracerStart = nullptr;
    Node* retracerParent = nullptr;
    //if currentNode is leafNode
    if(!currentNode->m_leftChild && !currentNode->m_rightChild)
    {
        if(parentNode)
        {
            if(parentNode->m_rightChild == currentNode)
            {
                parentNode->m_rightChild = nullptr;
                rebalanceNoChildNode(parentNode, false);
            }
            else
            {
                parentNode->m_leftChild = nullptr;
                rebalanceNoChildNode(parentNode, true);
            }

        }
        else
        {
            m_root = nullptr;
        }
    }
    //if currentNode has only left  or right child 
    else if( (currentNode->m_leftChild && !currentNode->m_rightChild) ||
                 (!currentNode->m_leftChild && currentNode->m_rightChild)) 
    {
        Node* replacement = currentNode->m_leftChild != nullptr ? currentNode->m_leftChild : currentNode->m_rightChild;
        replacement->m_parent = parentNode;
        if(parentNode)
        {
            if(parentNode->m_rightChild == currentNode)
            {
                parentNode->m_rightChild = replacement;
                rebalanceAfterDeletion(parentNode, parentNode->m_rightChild);
            }
            else
            {
                parentNode->m_leftChild = replacement;
                rebalanceAfterDeletion(parentNode, parentNode->m_leftChild);
            }
        }
        else
        {
            m_root = replacement;
        }

    }
    //if currentNode has both children
    else
    {
        
        Node* y = currentNode->m_rightChild;
        if(!y->m_leftChild)
        {
            //right child replaces current node
            y->m_parent = parentNode;
            y->m_leftChild = currentNode->m_leftChild;
            if( y->m_leftChild->m_parent)
            {
                y->m_leftChild->m_parent = y;
            }

            if(parentNode)
            {
                if(parentNode->m_rightChild == currentNode)
                {parentNode->m_rightChild = y;}
                else
                {parentNode->m_leftChild = y;}
            }
            else
            {
                m_root = y;
            }
            rebalanceRecursive(y, currentNode);
        }
        else
        {
            
            while (true)
            {
                if(!y->m_leftChild)
                {
                    break;
                }
                y = y->m_leftChild;

            }
            //update y parent node
            Node* R = y->m_parent;
            R->m_leftChild = y->m_rightChild;
            if(y->m_rightChild)
            {
                y->m_rightChild->m_parent =  R;
            }
            //use y to replace currentNode
            y->m_parent = currentNode->m_parent;
            if(currentNode->m_parent)
            {
                if(parentNode->m_rightChild == currentNode){parentNode->m_rightChild = y;}
                else{parentNode->m_leftChild = y;}
            }
            else
            {
                m_root = y;
            }

            y->m_leftChild = currentNode->m_leftChild;
            if(currentNode->m_leftChild)
            {
                currentNode->m_leftChild->m_parent =  y;
            }
            y->m_rightChild = currentNode->m_rightChild;
            if(currentNode->m_rightChild)
            {
                y->m_rightChild->m_parent =  y;
            }
            
            y->m_balanceFactor = currentNode->m_balanceFactor;
            rebalanceAfterDeletion(R, R->m_leftChild);
        }

    } 

    delete currentNode;
    currentNode = nullptr;

    return true;
}

//TODO
int AvlTree::addRefrenceToRow(char *data, char *key)
{
    return 0;
}

void AvlTree::clear()
{
    queue<Node*> nodes;
    nodes.push(m_root);
    while (nodes.size() > 0 )
    {
        Node* current = nodes.front();
        nodes.pop();

        if(current->m_leftChild )
        {
            nodes.push(current->m_leftChild);
        }
        if(current->m_rightChild )
        {
            nodes.push(current->m_rightChild);
        }
        delete current;
    }
    
    m_root = nullptr;
}

//return 0 if values is found, otherwise return 1
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
            (*node)->m_balanceFactor = 0;
            break;
        }

        int comp = (*node)->m_key_int - *(int*)key;
        if(comp == 0 )
        {
            return 0;
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

            parent->m_balanceFactor--;
            
            // left heavy, needs rebalance
            if(parent->m_balanceFactor == -2)
            {
                // right right rotation
                if(child->m_balanceFactor <= 0 )
                {
                    rightRightBalance(parent);
                    break;
                }
                // left right rotation
                else
                {
                    leftRightBalance(parent);
                    break;
                }   
            }

            if(parent->m_balanceFactor == 0)
            {
                // this means that added node rebalanced parent tree, 
                // so no change in height for upstream nodes
                break;
            }
        }
        else
        {
            parent->m_balanceFactor++;
            // right heavy, needs rebalance
            if(parent->m_balanceFactor == 2)
            {
                // right left rotation
                if(child->m_balanceFactor < 0 )
                {
                    rightLeftBalance(parent);
                    break;
                }
                // left left rotation
                else
                {
                    leftLeftBalance(parent);
                    break;
                }
            }

            if(parent->m_balanceFactor == 0)
            {
                // this means that added node rebalanced parent tree, 
                // so no change in height for upstream nodes
                break;
            }
        }


        child = parent;
        parent = child->m_parent;
    }
    return 1;
}

Node* AvlTree::leftLeftBalance(Node* x)
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
        m_root = z;
    }
    x->m_parent =z;
    // connecting right child
    x->m_rightChild = holder;
    if(holder)
    {
        holder->m_parent = x;
    }
    if( z->m_balanceFactor == 0)
    {
        z->m_balanceFactor = -1;
        x->m_balanceFactor = 1;
    }
    else
    {
        z->m_balanceFactor = 0;
        x->m_balanceFactor = 0;
    }
  
    return z;
}

Node* AvlTree::rightRightBalance(Node *x)
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
    // connecting left child
    x->m_leftChild = holder;
    if(holder)
    {
        holder->m_parent = x;
    }

    if (z->m_balanceFactor == 0)
    { 
        z->m_balanceFactor = 1;
        x->m_balanceFactor = -1;
    }
    else
    {
        z->m_balanceFactor = 0;
        x->m_balanceFactor = 0;
    }
    return z;
}

Node* AvlTree::rightLeftBalance(Node *x)
{
    Node* z = x->m_rightChild;
    Node* y = z->m_leftChild;

    //first swap 
    Node* holder_y = y->m_rightChild;
    y->m_rightChild = z;
    y->m_parent = z->m_parent;
    z->m_parent = y;
    z->m_leftChild  = holder_y;
    if(holder_y)
    {
        holder_y->m_parent = z;
    }
    // second swap
    holder_y = y->m_leftChild;
    y->m_leftChild = x;
    y->m_parent = x->m_parent;
    if(x->m_parent)
    {
        //update parent pointer
        Node* parent = x->m_parent;
        if(parent->m_leftChild == x) {parent->m_leftChild = y;} 
        else{parent->m_rightChild = y;}
    }
    else
    {
        m_root = y;
    }
    x->m_parent = y;
    x->m_rightChild = holder_y;
    if(holder_y)
    {
        holder_y->m_parent = x;
    }
    // find balance factors
    if(y->m_balanceFactor == 0)
    {
        z->m_balanceFactor = 0;
        x->m_balanceFactor = 0;
    }
    else if(y->m_balanceFactor > 0 )
    {
        z->m_balanceFactor = 0;
        x->m_balanceFactor = -1;
    }
    else
    {
        z->m_balanceFactor = 1;
        x->m_balanceFactor = 0;
    }
    y->m_balanceFactor = 0;
    return y;
}

Node* AvlTree::leftRightBalance(Node *x)
{
    Node* z = x->m_leftChild;
    Node* y = z->m_rightChild;

    //first swap
    Node* holder_y = y->m_leftChild;

    y->m_leftChild = z;
    y->m_parent = z->m_parent;
    z->m_parent = y;
    z->m_rightChild = holder_y;
    if(holder_y)
    {
        holder_y->m_parent = z;
    }
    // second swap
    holder_y = y->m_rightChild;

    y->m_rightChild = x;
    y->m_parent = x->m_parent;
    if(x->m_parent)
    {
        //update parent pointer
        Node* parent = x->m_parent;
        if(parent->m_leftChild == x) {parent->m_leftChild = y;} 
        else{parent->m_rightChild = y;}
    }
    else
    {
        m_root = y;
    }
    x->m_parent = y;
    x->m_leftChild = holder_y;
    if(holder_y)
    {
        holder_y->m_parent = x;
    }
    // find balance factors
    if(y->m_balanceFactor == 0)
    {
        z->m_balanceFactor = 0;
        x->m_balanceFactor = 0;
    }
    else if(y->m_balanceFactor > 0 )
    {
        z->m_balanceFactor = -1;
        x->m_balanceFactor = 0;
    }
    else
    {
        z->m_balanceFactor = 0;
        x->m_balanceFactor = 1;
    }
    y->m_balanceFactor = 0;
    return y;
}

void AvlTree::rebalanceNoChildNode(Node *y, bool childIsLeftNode)
{
    //if node CANNOT be unbalanced update only balance factor
    if(y->m_balanceFactor == 0) 
    {
        if(childIsLeftNode){y->m_balanceFactor++;}
        else{y->m_balanceFactor--;}

        return;
    }
    Node* child = childIsLeftNode ? y->m_leftChild : y->m_rightChild;
    //both nodes are null so no need to process y
    if(!y->m_leftChild && !y->m_rightChild)
    {
        rebalanceAfterDeletion(y->m_parent, y);
    }
    else
    {
        rebalanceAfterDeletion(y, child);
    }
}

void AvlTree::rebalanceRecursive(Node *y, Node *oldY)
{

    if(oldY->m_balanceFactor == 1)
    {
        y->m_balanceFactor = 0;
        rebalanceAfterDeletion(y->m_parent, y);
    }
    else if(oldY->m_balanceFactor == -1)
    {
        //pass right child to indicate that we lost unit of height in right subtree
        y->m_balanceFactor = -1;
        rebalanceAfterDeletion(y, y->m_rightChild);
    }
    else
    {
        y->m_balanceFactor = -1;
    }
}

// child indicates subtree from which we lost 1 unit of height
void AvlTree::rebalanceAfterDeletion(Node *parent, Node *child)
{
    while (parent)
    {
        if(parent->m_leftChild == child)
        {
            parent->m_balanceFactor++;
            if(parent->m_balanceFactor == 2)
            {
                if(parent->m_rightChild->m_balanceFactor < 0 )
                {
                    parent = rightLeftBalance(parent);
                }
                else
                {   
                    parent = leftLeftBalance(parent);
                }
            }
        }
        else
        {
            parent->m_balanceFactor--;
            if(parent->m_balanceFactor == -2)
            {
                if(parent->m_leftChild->m_balanceFactor  > 0 )
                {
                    parent = leftRightBalance(parent);
                }
                else
                {   
                    parent = rightRightBalance(parent);
                }
            }
        }

        if(parent->m_balanceFactor != 0)
        {
            // stop only if parent is non zero
            // if it is 0 then height of sutree descreased
            break;
        }

        child = parent;
        parent = child->m_parent;
    }
}
