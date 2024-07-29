#define TEST_AVL_TREE
#include "../dbms/algorithms/avl_tree.hpp"
#include <random>
#include <array>
#include <algorithm>
#include <limits>
#include <queue>
#include <stack>
#define ELEMENT_COUNT 3000
#define TEST_COUNT 20
#define NUMBER_BOUNDARY 6000
using namespace std;

struct TestEntry
{
    Node* node;
    int upperBound;
    int lowerBound;
};

bool checkIfIsBinaryTree(array<int, ELEMENT_COUNT> &arr, AvlTree *tree);
void checkIfValuesMatch(array<int, ELEMENT_COUNT> &arr, Node* node, int& index);

int main()
{

    if(NUMBER_BOUNDARY * 2 <  ELEMENT_COUNT)
    {
        printf("Incorrect array sizes for the test\n");
        exit(-1);
    }

    AvlTree tree(MachineDataTypes::INT32, 4);

    random_device rd;  // a seed source for the random number engine
    mt19937_64  gen{rd()}; // mersenne_twister_engine seeded with rd()
    array<int, NUMBER_BOUNDARY* 2> sourceValues;

    for(int i=0; i < NUMBER_BOUNDARY * 2; i++)
    {
        sourceValues[i] = i - NUMBER_BOUNDARY;
    }

    array<int, ELEMENT_COUNT> arr;

    int passedCount = 0;
    for(int i=0; i < TEST_COUNT; i++ )
    {
        shuffle(sourceValues.begin(), sourceValues.end(), gen);
        for(int i=0; i < arr.size(); i++)
        {
            arr[i] = sourceValues[i];
        }

        printf("test number %d ", i);
        for(int i = 0; i < arr.size() ; i++)
        {
            tree.insertValue((char*)&arr[i]);
        }
        
        if(checkIfIsBinaryTree(arr, &tree))
        {
            printf("is passed\n");
            passedCount++;
        }
        else
        {
            printf("is not passed\n");
        }
        tree.clear();
    }
    printf("In summary %d out of %d tests were passed\n", passedCount, TEST_COUNT);
    int x = 2;
}

bool checkIfIsBinaryTree(array<int, ELEMENT_COUNT> &arr, AvlTree *tree)
{
    sort( arr.begin(), arr.end() );
    vector<int> elementsFromTree;
    int upperBound = numeric_limits<int>::max();
    int lowerBound = numeric_limits<int>::min();


    queue<TestEntry> entries;
    entries.push({tree->m_root, upperBound, lowerBound});
    // check if tree satisfies binary tree property 
    while (entries.size() > 0 )
    {   

        TestEntry entry = entries.front();
        Node* currentNode = entry.node;
        if(currentNode->m_key_int > entry.upperBound || currentNode->m_key_int  < entry.lowerBound )
        {
            return false;
        }
        entries.pop();
        if(currentNode->m_leftChild)
        {
            entries.push({currentNode->m_leftChild, (int)currentNode->m_key_int, entry.lowerBound});
        }
        if( currentNode->m_rightChild)
        {
            entries.push({currentNode->m_rightChild, entry.upperBound, (int)currentNode->m_key_int});
        }
    }
    
    stack<Node*> nodes;
    array<int, ELEMENT_COUNT> buffer;
    nodes.push(tree->m_root);
    int i = 0;
    checkIfValuesMatch(buffer, tree->m_root, i);
    for(int index =0; index < ELEMENT_COUNT; index++)
    {
        if(arr[index]!= buffer[index])
        {   
            printf("error values are %d, %d != %d\n", index, arr[index], buffer[index]);
            return false;
        }
    }


    return true;
}

void checkIfValuesMatch(array<int, ELEMENT_COUNT> &arr, Node* node, int& index)
{

    if(!node)
    {
        return;
    }

    checkIfValuesMatch(arr, node->m_leftChild, index);

    arr[index ] = node->m_key_int;
    index++;

    return checkIfValuesMatch(arr, node->m_rightChild, index);
}