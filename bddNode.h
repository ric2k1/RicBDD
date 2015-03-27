/****************************************************************************
  FileName     [ bddNode.h ]
  PackageName  [ ]
  Synopsis     [ Define basic BDD Node data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BDD_NODE_H
#define BDD_NODE_H

#include <vector>
#include <map>
#include <iostream>

using namespace std;

#define BDD_EDGE_BITS      2
//#define BDD_NODE_PTR_MASK  ((UINT_MAX >> BDD_EDGE_BITS) << BDD_EDGE_BITS)
#define BDD_NODE_PTR_MASK  ((~(size_t(0)) >> BDD_EDGE_BITS) << BDD_EDGE_BITS)

class BddMgr;
class BddNodeInt;

enum BDD_EDGE_FLAG
{
   BDD_POS_EDGE = 0,
   BDD_NEG_EDGE = 1,

   BDD_EDGE_DUMMY  // dummy end
};

class BddNode
{
public:
   static BddNode          _one;
   static BddNode          _zero;
   static bool             _debugBddAddr;
   static bool             _debugRefCount;

   // no node association yet
   BddNode() : _node(0) {}
   // We check the hash when a new node is possibly being created
   BddNode(size_t l, size_t r, size_t i, BDD_EDGE_FLAG f = BDD_POS_EDGE);
   // Copy constructor also needs to increase the _refCount
   BddNode(const BddNode& n);
   // n must have been uniquified...
   BddNode(BddNodeInt* n, BDD_EDGE_FLAG f = BDD_POS_EDGE);
   // 
   BddNode(size_t v);
   // Destructor is the only place to decrease _refCount
   ~BddNode();

   // Basic access functions
   const BddNode& getLeft() const;
   const BddNode& getRight() const;
   BddNode getLeftCofactor(unsigned i) const;
   BddNode getRightCofactor(unsigned i) const;
   unsigned getLevel() const;
   unsigned getRefCount() const;
   bool isNegEdge() const { return (_node & BDD_NEG_EDGE); }
   bool isPosEdge() const { return !isNegEdge(); }

   // Operators overloading
   size_t operator () () const { return _node; }
   BddNode operator ~ () const { return (_node ^ BDD_NEG_EDGE); }
   BddNode& operator = (const BddNode& n);
   BddNode operator & (const BddNode& n) const;
   BddNode& operator &= (const BddNode& n);
   BddNode operator | (const BddNode& n) const;
   BddNode& operator |= (const BddNode& n);
   BddNode operator ^ (const BddNode& n) const;
   BddNode& operator ^= (const BddNode& n);
   bool operator == (const BddNode& n) const { return (_node == n._node); }
   bool operator != (const BddNode& n) const { return (_node != n._node); }
   bool operator < (const BddNode& n) const;
   bool operator <= (const BddNode& n) const;
   bool operator > (const BddNode& n) const { return !((*this) <= n); }
   bool operator >= (const BddNode& n) const { return !((*this) < n); }

   // Other BDD operations
   BddNode exist(unsigned l) const;
   BddNode nodeMove(unsigned fLevel, unsigned tLevel, bool& isMoved) const;
   size_t countCube() const;
   BddNode getCube(size_t ith=0) const;
   vector<BddNode> getAllCubes() const;
   string toString() const;

   friend ostream& operator << (ostream& os, const BddNode& n);

   // For BDD drawing
   void drawBdd(const string&, ofstream&) const;
   string getLabel() const;

   // Static functions
   static void setBddMgr(BddMgr* m) { _BddMgr = m; }

private:
   size_t                  _node;

   // Static data mebers
   static BddMgr*          _BddMgr;

   // Private functions
   BddNodeInt* getBddNodeInt() const {
      return (BddNodeInt*)(_node & BDD_NODE_PTR_MASK); }
   bool isTerminal() const;
   void print(ostream&, size_t, size_t&) const;
   void unsetVisitedRecur() const;
   void drawBddRecur(ofstream&) const;
   // comment out for SoCV BDD
   BddNode existRecur(unsigned l, map<size_t, size_t>&) const;
   BddNode nodeMoveRecur(unsigned f, unsigned t, map<size_t, size_t>&) const;
   bool containNode(unsigned b, unsigned e) const;
   bool containNodeRecur(unsigned b, unsigned e) const;
   size_t countCubeRecur(bool phase, map<size_t, size_t>& numCubeMap) const;
   bool getCubeRecur(bool p, size_t& ith, size_t target, BddNode& res) const;
   void getAllCubesRecur(bool p, BddNode& c, vector<BddNode>& aCubes) const;
   bool toStringRecur(bool p, string& str) const;
};

// Private class
class BddNodeInt
{
   friend class BddNode;
   friend class BddMgr;

   // For const 1 and const 0
   BddNodeInt() : _level(0), _refCount(0), _visited(0) {}

   // Don't initialize _refCount here...
   // BddNode() will call incRefCount() or decRefCount() instead...
   BddNodeInt(size_t l, size_t r, unsigned ll)
   : _left(l), _right(r), _level(ll), _refCount(0), _visited(0) {}

   const BddNode& getLeft() const { return _left; }
   const BddNode& getRight() const { return _right; }
   unsigned getLevel() const { return _level; }
   unsigned getRefCount() const { return _refCount; }
   void incRefCount() { ++_refCount; }
   void decRefCount() { --_refCount; }
   bool isVisited() const { return (_visited == 1); }
   void setVisited() { _visited = 1; }
   void unsetVisited() { _visited = 0; }

   BddNode              _left;
   BddNode              _right;
   unsigned             _level    : 16;
   unsigned             _refCount : 15;
   unsigned             _visited  : 1;

   static BddNodeInt*   _terminal;
};

#endif // BDD_NODE_H
