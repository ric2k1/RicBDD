/****************************************************************************
  FileName     [ bddMgr.h ]
  PackageName  [ ]
  Synopsis     [ Define BDD Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BDD_MGR_H
#define BDD_MGR_H

#include <map>
#include "myHash.h"
#include "bddNode.h"

using namespace std;

class BddNode;

typedef vector<size_t>                        BddArr;
typedef map<string, size_t>                   BddMap;
typedef pair<string, size_t>                  BddMapPair;
typedef map<string, size_t>::const_iterator   BddMapConstIter;

extern BddMgr* bddMgr;

class BddHashKey
{
public:
   // TODO: define constructor(s)
   BddHashKey(size_t l, size_t r, unsigned i) : _l(l), _r(r), _i(i) {}

   // TODO: implement "()" and "==" operators
   // Get a size_t number;
   // ==> to get bucket number, need to % _numBuckers in Hash
   size_t operator() () const { return ((_l << 3) + (_r << 3) + _i); }

   bool operator == (const BddHashKey& k) {
      return (_l == k._l) && (_r == k._r) && (_i == k._i); }

private:
   // TODO: define your own data members
   size_t     _l;
   size_t     _r;
   unsigned   _i;
};

class BddCacheKey
{
public:
   // TODO: define constructor(s)
   BddCacheKey() {}
   BddCacheKey(size_t f, size_t g, size_t h) : _f(f), _g(g), _h(h) {}

   // TODO: implement "()" and "==" operators
   // Get a size_t number;
   // ==> to get cache address, need to % _size in Cache
   size_t operator() () const { return ((_f << 3)+(_g << 3)+(_h << 3)); }

   bool operator == (const BddCacheKey& k) const {
      return (_f == k._f) && (_g == k._g) && (_h == k._h); }

private:
   // TODO: define your own data members
   size_t       _f;
   size_t       _g;
   size_t       _h;
};

class BddMgr
{
typedef Hash<BddHashKey, BddNodeInt*> BddHash;
typedef Cache<BddCacheKey, size_t>    BddCache;

public:
   BddMgr(size_t nin = 64, size_t h = 8009, size_t c = 30011)
   { init(nin, h, c); }
   ~BddMgr() { reset(); }

   void init(size_t nin, size_t h, size_t c);
   void restart();

   // for building BDDs
   BddNode ite(BddNode f, BddNode g, BddNode h);

   // for _supports
   const BddNode& getSupport(size_t i) const { return _supports[i]; }
   size_t getNumSupports() const { return _supports.size(); }

   // for _uniqueTable
   BddNodeInt* uniquify(size_t l, size_t r, unsigned i);

   // for _bddArr: access by unsigned (ID)
   bool addBddNode(unsigned id, size_t nodeV);
   BddNode getBddNode(unsigned id) const;
 
   // for _bddMap: access by string
   bool addBddNode(const string& nodeName, size_t nodeV);
   void forceAddBddNode(const string& nodeName, size_t nodeV);
   BddNode getBddNode(const string& nodeName) const;

   // Applications
   int evalCube(const BddNode& node, const string& vector) const;
   bool drawBdd(const string& nodeName, const string& dotFile) const;

private:
   // level = 0: const 1;
   // level = 1: lowest input variable
   // level = nin: highest input variable
   vector<BddNode>  _supports;
   BddHash          _uniqueTable;
   BddCache         _computedTable;

   BddArr           _bddArr;
   BddMap           _bddMap;

   void reset();
   bool checkIteTerminal(const BddNode&, const BddNode&, const BddNode&,
                         BddNode&);
   void standardize(BddNode &f, BddNode &g, BddNode &h, bool &isNegEdge);
};

#endif // BDD_MGR_H
