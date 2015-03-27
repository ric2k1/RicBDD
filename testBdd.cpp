/****************************************************************************
  FileName     [ testBdd.cpp ]
  PackageName  [ ]
  Synopsis     [ Define main() ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "bddNode.h"
#include "bddMgr.h"

using namespace std;

/**************************************************************************/
/*                        Define Global BDD Manager                       */
/**************************************************************************/
BddMgr bm;

/**************************************************************************/
/*                    Define Static Function Prototypes                   */
/**************************************************************************/
static void initBdd(size_t nSupports, size_t hashSize, size_t cacheSize);


/**************************************************************************/
/*                             Define main()                              */
/**************************************************************************/
int
main()
{
   initBdd(5, 127, 61);

   /*-------- THIS IS JUST A TEST CODE ---------*/
   BddNode a(bm.getSupport(1));
   BddNode b(bm.getSupport(2));
   BddNode c(bm.getSupport(3));
   BddNode d(bm.getSupport(4));
   BddNode e(bm.getSupport(5));

   BddNode f = ~(a & b);
   cout << f << endl;

   BddNode g = c | d;
   cout << g << endl;

   BddNode h = ~e;
   cout << h << endl;

   BddNode i = f ^ (c | d); // f ^ g;
   cout << i << endl;

   BddNode j = ~a | ~b;
   cout << j << endl;

   cout << "KK" << endl;
   BddNode k = ( (a|b) ^ (d&e) );
   cout << k << endl;
//   cout << k.getLeftCofactor(1) << endl;
   cout << k.getLeftCofactor(2) << endl;
//   cout << k.getRightCofactor(4) << endl;
   cout << k.getRightCofactor(2) << endl;
   BddNode l1 = k.exist(2);
   cout << l1 << endl;
   BddNode l2 = k.getLeftCofactor(2) | k.getRightCofactor(2);
   cout << l2 << endl;

   ofstream ofile("i.dot");
   i.drawBdd("i", ofile);
   system("dot -o i.png -Tpng i.dot");

   /*----------- END OF TEST CODE ------------*/
}


/**************************************************************************/
/*                          Define Static Functions                       */
/**************************************************************************/
static void
initBdd(size_t nin, size_t h, size_t c)
{
   BddNode::_debugBddAddr = true;
   BddNode::_debugRefCount = true;

//   bm.reset();
   bm.init(nin, h, c);
}

