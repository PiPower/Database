#define TEST_AVL_TREE
#include "../dbms/algorithms/avl_tree.hpp"
#include <random>
#include <array>
#include <algorithm>
#include <limits>
#include <queue>
#include <stack>
#define ELEMENT_COUNT 500
#define TEST_COUNT 1000
#define NUMBER_BOUNDARY 500
using namespace std;

struct TestEntry
{
    Node* node;
    int upperBound;
    int lowerBound;
};

bool checkIfIsBinaryTree(vector<int> &arr, AvlTree *tree);
void checkIfValuesMatch(vector<int>&arr, Node* node, int& index);
int getSubTreeSize(Node* node);

int main()
{

    if(NUMBER_BOUNDARY * 2 <  ELEMENT_COUNT)
    {
        printf("Incorrect array sizes for the test\n");
        exit(-1);
    }

    AvlTree tree(MachineDataTypes::INT32, 4);

    random_device rd;
    mt19937_64  gen{rd()};
    array<int, NUMBER_BOUNDARY* 2> values;
    uniform_int_distribution<int> dist(ELEMENT_COUNT * 0.6 , ELEMENT_COUNT * 0.8);

    for(int i=0; i < NUMBER_BOUNDARY * 2; i++)
    {
        values[i] = i - NUMBER_BOUNDARY;
    }

    vector<int> arr;
    vector<int> buffer;
    arr.resize(ELEMENT_COUNT);
    int insertionPassedCount = 0;
    int deletionPassedCount = 0;
    for(int i=0; i < TEST_COUNT; i++ )
    {
        shuffle(values.begin(), values.end(), gen);
        for(int i=0; i < ELEMENT_COUNT; i++)
        {
            arr[i] = values[i];
        }
        printf("Test number %d. ", i);
        for(int i = 0; i < arr.size() ; i++)
        {
            tree.insertValue((char*)&arr[i]);
        }
        //insertion test
        if(checkIfIsBinaryTree(arr, &tree))
        {
            printf("Insertion test has been passed. ");
            insertionPassedCount++;
        }
        else
        {
            printf("Insertion test has not been passed. ");
        }
        // Removal test------------------------------------------
        int remainingElementCount = dist(gen);
        shuffle(values.begin(), values.begin() + ELEMENT_COUNT, gen);
        int j = 0;
        buffer.resize(remainingElementCount);
        for(j =0; j < remainingElementCount; j++)
        {
            buffer[j] = values[j];
        }

        for(int p = j;p < ELEMENT_COUNT; p++)
        {
            tree.removeValue( (char*)&values[p] );
        }
        //deletion test
        if(checkIfIsBinaryTree(buffer, &tree))
        {
            printf("Deletion test has been passed. \n");
            deletionPassedCount++;
        }
        else
        {
            printf("Deletion test has not been passed. \n");
        }
        tree.clear();
    }
    printf("In summary out of %d tests, %d were passed for insertion and %d for deletion\n", TEST_COUNT, insertionPassedCount, deletionPassedCount);
    int x = 2;
}

bool checkIfIsBinaryTree(vector<int>&arr, AvlTree *tree)
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
    // check if values are placed correctly
    stack<Node*> nodes;
    vector<int>buffer;
    buffer.resize(arr.size());
    nodes.push(tree->m_root);
    int i = 0;
    checkIfValuesMatch(buffer, tree->m_root, i);
    for(int index =0; index < arr.size(); index++)
    {
        if(arr[index]!= buffer[index])
        {   
            printf("error values are %d, %d != %d\n", index, arr[index], buffer[index]);
            return false;
        }
    }
    //check if tree is balance 
    int l_h = getSubTreeSize(tree->m_root->m_leftChild);
    int r_h = getSubTreeSize(tree->m_root->m_rightChild);
    if(abs(l_h - r_h ) >= 2)
    {
        return false;
    }
    return true;
}

void checkIfValuesMatch(vector<int> &arr, Node* node, int& index)
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

int getSubTreeSize(Node *node)
{
    if(!node)
    {
        return 0;
    }

    int l_h = getSubTreeSize(node->m_leftChild);
    int r_h = getSubTreeSize(node->m_rightChild);
    return max(l_h, r_h) + 1;
}
