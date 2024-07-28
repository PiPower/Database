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
    array<int, 6> arr = { 7, 8, 0,68,11, 93};
    //ranges::generate(arr, [&](){  return distrib(gen);} );
    for(int i = 0; i < arr.size() ; i++)
    {
        tree.insertValue((char*)&arr[i]);
    }
    int x = 2;
}