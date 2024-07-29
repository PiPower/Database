#include "../dbms/algorithms/avl_tree.hpp"
#include <random>
#include <array>
#include <algorithm>
using namespace std;


int main()
{
    AvlTree tree(MachineDataTypes::INT32, 4);

    random_device rd;  // a seed source for the random number engine
    mt19937 gen{rd()}; // mersenne_twister_engine seeded with rd()
    uniform_int_distribution<int> distrib(1, 50000);
    array<int, 9> arr = { 8,-5, 1,-3, 68 ,-10,-20, -7, -6};
    //ranges::generate(arr, [&](){  return distrib(gen);} );
    for(int i = 0; i < arr.size() ; i++)
    {
        int x = arr[i] + 20;
        tree.insertValue((char*)&x);
    }
    int x = 2;
}