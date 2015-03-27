/****************************************************************************
  FileName     [ bddMgr.cpp ]
  PackageName  [ ]
  Synopsis     [ BDD Manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iomanip>
#include <cassert>
#include "bddNode.h"
#include "bddMgr.h"

using namespace std;

//----------------------------------------------------------------------
//    Global variables
//----------------------------------------------------------------------
BddMgr* bddMgr = new BddMgr;

//----------------------------------------------------------------------
//    External functions
//----------------------------------------------------------------------
bool myStr2Int(const string&, int&);

//----------------------------------------------------------------------
//    static functions
//----------------------------------------------------------------------
static void swapBddNode(BddNode& f, BddNode& g)
{
   BddNode tmp = f; f = g; g = tmp;
}

//----------------------------------------------------------------------
//    helper functions
//----------------------------------------------------------------------
BddNode getBddNode(const string& bddName)
{
   int id;
   if (myStr2Int(bddName, id))  // bddName is an ID
      return bddMgr->getBddNode(id);
   else // bddNameis a name
      return bddMgr->getBddNode(bddName);
}

//----------------------------------------------------------------------
//    class BddMgr
//----------------------------------------------------------------------
// _level = 0 ==> const 1 & const 0
// _level = 1 ~ nin ==> supports
//
void
BddMgr::init(size_t nin, size_t h, size_t c)
{
   reset();
   _uniqueTable.init(h);
   _computedTable.init(c);

   // This must be called first
   BddNode::setBddMgr(this);
   BddNodeInt::_terminal = uniquify(0, 0, 0);
   BddNode::_one = BddNode(BddNodeInt::_terminal, BDD_POS_EDGE);
   BddNode::_zero = BddNode(BddNodeInt::_terminal, BDD_NEG_EDGE);

   _supports.reserve(nin+1);
   _supports.push_back(BddNode::_one);
   for (size_t i = 1; i <= nin; ++i)
      _supports.push_back(BddNode(BddNode::_one(), BddNode::_zero(), i));
}

// Called by the CIRSETVar command
void
BddMgr::restart()
{
   size_t nin = _supports.size() - 1;
   size_t h   = _uniqueTable.numBuckets();
   size_t c   = _computedTable.size();

   init(nin, h, c);
}

// This is a private function called by init() and restart()
void
BddMgr::reset()
{
   // TODO
   _supports.clear();
   _bddArr.clear();
   _bddMap.clear();
   BddHash::iterator bi = _uniqueTable.begin();
   for (; bi != _uniqueTable.end(); ++bi)
      delete (*bi).second;
   _uniqueTable.reset();
   _computedTable.reset();
}

// [Note] Remeber to check "isNegEdge" when return BddNode!!!!!
//
BddNode
BddMgr::ite(BddNode f, BddNode g, BddNode h)
{
   bool isNegEdge = false;  // should only be flipped by "standardize()"

#define DO_STD_ITE 1  // NOTE: make it '0' if you haven't done standardize()!!
   standardize(f, g, h, isNegEdge);

   BddNode ret;

   // check terminal cases
   if (checkIteTerminal(f, g, h, ret)) {
      if (isNegEdge) ret = ~ret;
      return ret;  // no need to update tables
   }

   // check computed table
   // TODO: based on your definition of BddCacheKey,
   //       instantiate a BddCacheKey k (i.e. pass in proper data members)
   // BddCacheKey k;  // Change this line!!
   BddCacheKey k(f(), g(), h());
   size_t ret_t;
   if (_computedTable.read(k, ret_t)) {
      if (isNegEdge) ret_t = ret_t ^ BDD_NEG_EDGE;
      return ret_t;
   }

   // check top varaible
   unsigned v = f.getLevel();
   if (g.getLevel() > v)
      v = g.getLevel();
   if (h.getLevel() > v)
      v = h.getLevel();

   // recursion
   BddNode fl = f.getLeftCofactor(v),
           gl = g.getLeftCofactor(v),
           hl = h.getLeftCofactor(v);
   BddNode t = ite(fl, gl, hl);

   BddNode fr = f.getRightCofactor(v),
           gr = g.getRightCofactor(v),
           hr = h.getRightCofactor(v);
   BddNode e = ite(fr, gr, hr);

   // get result
   if (t == e) {
      // update computed table
      _computedTable.write(k, t());
      if (isNegEdge) t = ~t;
      return t;
   }

   // move bubble if necessary... ==> update isNedEdge
   bool moveBubble = t.isNegEdge();
#if DO_STD_ITE
   assert(!moveBubble);
#else
   if (moveBubble) {
      t = ~t; e = ~e;
   }
#endif

   // check unique table
   BddNodeInt* ni = uniquify(t(), e(), v);
   ret_t = size_t(ni);
#if !(DO_STD_ITE)
   if (moveBubble) ret_t = ret_t ^ BDD_NEG_EDGE;
#endif
   // update computed table
   _computedTable.write(k, ret_t);
   if (isNegEdge)
      ret_t = ret_t ^ BDD_NEG_EDGE;
   return ret_t;
}

void
BddMgr::standardize(BddNode &f, BddNode &g, BddNode &h, bool &isNegEdge)
{
   // TODO
   // (1) Identical/Complement rules
   if (f == g) g = BddNode::_one;
   else if (f == ~g) g = BddNode::_zero;
   else if (f == h) h = BddNode::_zero;
   else if (f == ~h) h = BddNode::_one;

   // (2) Symmetrical rules
   if (g == BddNode::_one) {
      if (f > h) swapBddNode(f, h);
   }
   else if (g == BddNode::_zero) {
      if (f > h) { swapBddNode(f, h); f = ~f; h = ~h; }
   }
   else if (h == BddNode::_one) {
      if (f > g) { swapBddNode(f, g); f = ~f; g = ~g; }
   }
   else if (h == BddNode::_zero) {
      if (f > g) swapBddNode(f, g);
   }
   else if (g == ~h) {
      if (f > g) { swapBddNode(f, g); h = ~g; }
   }

   // (3) Complement edge rules
   // ==> both f and g will be posEdge afterwards
   if (f.isNegEdge()) { swapBddNode(g, h); f = ~f; }
   if (g.isNegEdge()) {
      g = ~g; h = ~h; isNegEdge = !isNegEdge;
   }
}

// Check if triplet (l, r, i) is in _uniqueTable,
// If not, create a new node;
// else, return the hashed one
//
BddNodeInt*
BddMgr::uniquify(size_t l, size_t r, unsigned i)
{
   // TODO
   BddNodeInt* n = 0;
   BddHashKey k(l, r, i);
   if (!_uniqueTable.check(k, n)) {
      n = new BddNodeInt(l, r, i);
      _uniqueTable.forceInsert(k, n);
   }
   return n;
}

// return false if _bddArr[id] has aleady been inserted
bool
BddMgr::addBddNode(unsigned id, size_t n)
{
   if (id >= _bddArr.size()) {
      unsigned origSize = _bddArr.size();
      _bddArr.resize(id+1);
      for(unsigned i = origSize; i < _bddArr.size(); ++i)
        _bddArr[i] = 0;
   } else if (_bddArr[id] != 0)
      return false;
   _bddArr[id] = n;
   return true;
}

// return 0 if not in the map!!
BddNode
BddMgr::getBddNode(unsigned id) const
{
   if (id >= _bddArr.size())
      return size_t(0);
   return _bddArr[id];
}

// return false if str is already in the _bddMap!!
bool
BddMgr::addBddNode(const string& str, size_t n)
{
   return _bddMap.insert(BddMapPair(str, n)).second;
}

void
BddMgr::forceAddBddNode(const string& str, size_t n)
{
   _bddMap[str] = n;
}

// return 0 if not in the map!!
BddNode
BddMgr::getBddNode(const string& name) const
{
   BddMapConstIter bi = _bddMap.find(name);
   if (bi == _bddMap.end()) return size_t(0);
   return (*bi).second;
}

// return true if terminal case
bool
BddMgr::checkIteTerminal
(const BddNode& f, const BddNode& g, const BddNode& h, BddNode& n)
{
   // TODO
   if (g == h) { n = g; return true; }
   if (f == BddNode::_one) { n = g; return true; }
   if (f == BddNode::_zero) { n = h; return true; }
   if (g == BddNode::_one && h == BddNode::_zero) { n = f; return true; }
   return false;
}

//----------------------------------------------------------------------
//    Application functions
//----------------------------------------------------------------------
// pattern[0 ~ n-1] ==> _support[1 ~ n]
//
// ==> return -1 if pattern is not legal
// ==> return 0/1 for evaluated result
int 
BddMgr::evalCube(const BddNode& node, const string& pattern) const
{
   // TODO
   size_t v = node.getLevel();
   size_t n = pattern.size();
   if (n < v) {
      cerr << "Error: " << pattern << " too short!!" << endl;
      return -1;
   }

   BddNode next = node;
   for (int i = v - 1; i >= 0; --i) {
      char c = pattern[i];
      if (c == '1')
         next = next.getLeftCofactor(i+1);
      else if (c == '0')
         next = next.getRightCofactor(i+1);
      else {
         cerr << "Illegal pattern: " << c << "(" << i << ")" << endl;
         return -1;
      }
   }
   return (next == BddNode::_one)? 1 : 0;
}

bool
BddMgr::drawBdd(const string& name, const string& fileName) const
{
   // TODO
   BddNode node = ::getBddNode(name);
   if (node() == 0) {
      cerr << "Error: \"" << name << "\" is not a legal BDD node!!" << endl;
      return false;
   }

   ofstream ofile(fileName.c_str());
   if (!ofile) {
      cerr << "Error: cannot open file \"" << fileName << "\"!!" << endl;
      return false;
   }

   node.drawBdd(name, ofile);

   return true;
}
