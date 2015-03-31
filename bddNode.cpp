/****************************************************************************
  FileName     [ bddNode.cpp ]
  PackageName  [ ]
  Synopsis     [ Define BDD Node member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include "bddNode.h"
#include "bddMgr.h"

// Initialize static data members
//
BddMgr* BddNode::_BddMgr = 0;
BddNodeInt* BddNodeInt::_terminal = 0;
BddNode BddNode::_one;
BddNode BddNode::_zero;
bool BddNode::_debugBddAddr = false;
bool BddNode::_debugRefCount = false;

// We check the hash when a new BddNodeInt is possibly being created
BddNode::BddNode(size_t l, size_t r, size_t i, BDD_EDGE_FLAG f)
{
   BddNodeInt* n = _BddMgr->uniquify(l, r, i);
   // n should not = 0
   assert(n != 0);
   _node = size_t(n) + f;
   n->incRefCount();
}

// Copy constructor also needs to increase the _refCount
// Need to check if n._node != 0
BddNode::BddNode(const BddNode& n) : _node(n._node)
{
   BddNodeInt* t = getBddNodeInt();
   if (t)
      t->incRefCount();
}

// This function is called only when n is obtained from uniquify()...
// ==> n should not be "0"
BddNode::BddNode(BddNodeInt* n, BDD_EDGE_FLAG f)
{
   assert(n != 0);
   // TODO
   _node = size_t(n) + f;
   n->incRefCount();
}

// Need to check if n._node != 0
BddNode::BddNode(size_t v) : _node(v)
{
   // TODO
   BddNodeInt* n = getBddNodeInt();
   if (n)
      n->incRefCount();
}

// Need to check if _node != 0
BddNode::~BddNode()
{
   // TODO
   BddNodeInt* n = getBddNodeInt();
   if (n)
      n->decRefCount();
}

const BddNode&
BddNode::getLeft() const
{
   assert(getBddNodeInt() != 0);
   return getBddNodeInt()->getLeft();
}

const BddNode&
BddNode::getRight() const
{
   assert(getBddNodeInt() != 0);
   return getBddNodeInt()->getRight();
}

// [Note] i SHOULD NOT < getLevel()
BddNode
BddNode::getLeftCofactor(unsigned i) const
{
   assert(getBddNodeInt() != 0);
   assert(i > 0);
   // TODO
   if (i > getLevel()) return (*this);
   if (i == getLevel())
      return isNegEdge()? ~getLeft() : getLeft();
   BddNode t = getLeft().getLeftCofactor(i);
   BddNode e = getRight().getLeftCofactor(i);
   if (t == e) return isNegEdge()? ~t : t;
   BDD_EDGE_FLAG f = (isNegEdge() ^ t.isNegEdge())?
                      BDD_NEG_EDGE: BDD_POS_EDGE;
   if (t.isNegEdge()) { t = ~t; e = ~e; }
   return BddNode(t(), e(), getLevel(), f);
}

// [Note] i SHOULD NOT < getLevel()
BddNode
BddNode::getRightCofactor(unsigned i) const
{
   assert(getBddNodeInt() != 0);
   assert(i > 0);
   // TODO
   if (i > getLevel()) return (*this);
   if (i == getLevel())
      return isNegEdge()? ~getRight() : getRight();
   BddNode t = getLeft().getRightCofactor(i);
   BddNode e = getRight().getRightCofactor(i);
   if (t == e) return isNegEdge()? ~t : t;
   BDD_EDGE_FLAG f = (isNegEdge() ^ t.isNegEdge())?
                      BDD_NEG_EDGE: BDD_POS_EDGE;
   if (t.isNegEdge()) { t = ~t; e = ~e; }
   return BddNode(t(), e(), getLevel(), f);
}

unsigned
BddNode::getLevel() const
{
   return getBddNodeInt()->getLevel();
}

unsigned
BddNode::getRefCount() const
{
   return getBddNodeInt()->getRefCount();
}

// Note: BddNode a = b;
// ==> a's original BddNodeInt's reference count should --
//     a's new BddNodeInt's reference count should ++
BddNode&
BddNode::operator = (const BddNode& n)
{
   // TODO
   BddNodeInt* t = getBddNodeInt();
   if (t)
      t->decRefCount();
   _node = n._node;
   t = getBddNodeInt();
   if (t)
      t->incRefCount();
   return (*this);
}

BddNode
BddNode::operator & (const BddNode& n) const
{
   // TODO
   return _BddMgr->ite((*this), n, BddNode::_zero);
}

BddNode&
BddNode::operator &= (const BddNode& n)
{
   // TODO
   (*this) = (*this) & n;
   return (*this);
}

BddNode
BddNode::operator | (const BddNode& n) const
{
   // TODO
   return _BddMgr->ite((*this), BddNode::_one, n);
}

BddNode&
BddNode::operator |= (const BddNode& n)
{
   // TODO
   (*this) = (*this) | n;
   return (*this);
}

BddNode
BddNode::operator ^ (const BddNode& n) const
{
   // TODO
   return _BddMgr->ite((*this), ~n, n);
}

BddNode&
BddNode::operator ^= (const BddNode& n)
{
   // TODO
   (*this) = (*this) ^ n;
   return (*this);
}

// (*this) < n iff
// (i) level < n.level
// (ii) level == n.level && _node < n._node
bool
BddNode::operator < (const BddNode& n) const
{
   // TODO
   unsigned l1 = getLevel();
   unsigned l2 = n.getLevel();
   return ((l1 < l2) ||
          ((l1 == l2) && (_node < n._node)));
}

// (*this) < n iff
// (i) level < n.level
// (ii) level == n.level && _node <= n._node
bool
BddNode::operator <= (const BddNode& n) const
{
   // TODO
   unsigned l1 = getLevel();
   unsigned l2 = n.getLevel();
   return ((l1 < l2) ||
          ((l1 == l2) && (_node <= n._node)));
}

bool
BddNode::isTerminal() const
{
   return (getBddNodeInt() == BddNodeInt::_terminal);
}

// [BONUS TODO]... starting
//
BddNode
BddNode::exist(unsigned l) const
{
   if (l == 0) return (*this);

   map<size_t, size_t> existMap;
   return existRecur(l, existMap);
}

BddNode
BddNode::existRecur(unsigned l, map<size_t, size_t>& existMap) const
{
   if (isTerminal()) return (*this);

   unsigned thisLevel = getLevel();
   if (l > thisLevel) return (*this);

   map<size_t, size_t>::iterator mi = existMap.find(_node);
   if (mi != existMap.end()) return (*mi).second;

   BddNode left = getLeftCofactor(thisLevel);
   BddNode right = getRightCofactor(thisLevel);
   if (l == thisLevel) {
      BddNode res = left | right;
      existMap[_node] = res();
      return res;
   }

   // l must < getLevel()
   bool isNegEdge = false;
   BddNode t = left.existRecur(l, existMap);
   BddNode e = right.existRecur(l, existMap);
   if (t == e) {
      existMap[_node] = t();
      return t;
   }
   if (t.isNegEdge()) {
      t = ~t; e = ~e; isNegEdge = true;
   }
   BddNode res(_BddMgr->uniquify(t(), e(), thisLevel));
   if (isNegEdge) res = ~res;
   existMap[_node] = res();
   return res;
}

// Move the BDD nodes in the cone >= fromLevel to toLevel.
// After the move, there will be no BDD nodes between [fromLevel, toLevel).
// Return the resulted BDD node.
//
// [Note] If (fromLevel > toLevel) ==> move down
//        If (fromLevel < toLevel) ==> move up
//
// Before the move, need to make sure:
// 1. (thisLevel - fromLevel) < abs(fromLevel - toLevel)
// 2. There is no node < fromLevel (except for the terminal node).
//
// [Note] If (move up), there is no node above toLevel in the beginning.
//
// If any of the above is violated, isMoved will be set to false,
// no move will be made, and return (*this).
//
BddNode
BddNode::nodeMove(unsigned fromLevel, unsigned toLevel, bool& isMoved) const
{
   assert(fromLevel > 1);
   if (int(getLevel() - fromLevel) >= abs(int(fromLevel - toLevel)) ||
        containNode(fromLevel - 1, 1)) {
      isMoved = false;
      return (*this);
   }

   isMoved = true;
   map<size_t, size_t> moveMap;
   return nodeMoveRecur(fromLevel, toLevel, moveMap);
}

BddNode
BddNode::nodeMoveRecur
(unsigned fromLevel, unsigned toLevel, map<size_t, size_t>& moveMap) const
{
   unsigned thisLevel = getLevel();
   assert(thisLevel >= fromLevel);

   map<size_t, size_t>::iterator mi = moveMap.find(_node);
   if (mi != moveMap.end()) return (*mi).second;

   BddNode left = getLeft();
   BddNode right = getRight();

   if (!left.isTerminal())
      left = left.nodeMoveRecur(fromLevel, toLevel, moveMap);
   if (!right.isTerminal())
      right = right.nodeMoveRecur(fromLevel, toLevel, moveMap);

   BddNodeInt *n
   = _BddMgr->uniquify(left(), right(), thisLevel - fromLevel + toLevel);
   BddNode ret = BddNode(size_t(n));
   if (isNegEdge()) ret = ~ret;

   moveMap[_node] = ret();
   return ret;
}

// Check if there is any BddNode in the cone of level [bLevel, eLevel]
// return true if any
//
bool
BddNode::containNode(unsigned bLevel, unsigned eLevel) const
{
   bool res = containNodeRecur(bLevel, eLevel);
   unsetVisitedRecur();
   return res;
}

bool
BddNode::containNodeRecur(unsigned bLevel, unsigned eLevel) const
{
   BddNodeInt* n = getBddNodeInt();
   if (n->isVisited())
      return false;
   n->setVisited();

   unsigned thisLevel = getLevel();
   if (thisLevel < bLevel) return false;
   if (thisLevel <= eLevel) return true;

   if (getLeft().containNodeRecur(bLevel, eLevel)) return true;
   if (getRight().containNodeRecur(bLevel, eLevel)) return true;

   return false;
}     
   
size_t
BddNode::countCube() const
{  
   map<size_t, size_t> numCubeMap;
   return countCubeRecur(false, numCubeMap);
}  

size_t
BddNode::countCubeRecur(bool phase, map<size_t, size_t>& numCubeMap) const
{
   if (isTerminal())
      return ((phase ^ isNegEdge())? 0 : 1); 

   map<size_t, size_t>::iterator mi = numCubeMap.find(_node);
   if (mi != numCubeMap.end()) return (*mi).second;

   unsigned numCube = 0;
   BddNode left = getLeft();
   numCube += left.countCubeRecur(phase ^ isNegEdge(), numCubeMap);
   BddNode right = getRight();
   numCube += right.countCubeRecur(phase ^ isNegEdge(), numCubeMap);

   numCubeMap[_node] = numCube;
   return numCube;
}

BddNode
BddNode::getCube(size_t ith) const
{  
   ith %= countCube();
   BddNode res = BddNode::_one;
   size_t idx = 0;
   getCubeRecur(false, idx, ith, res);
   return res;
}
   
// return true if the target-th cube is met
bool
BddNode::getCubeRecur
(bool phase, size_t& ith, size_t target, BddNode& res) const
{
   if (isTerminal()) {
      if (!(phase ^ isNegEdge())) {
         if (ith == target) return true;
         ++ith;
      }
      return false;
   }

   BddNode old = res;
   BddNode left = getLeft();
   res = old & _BddMgr->getSupport(getLevel());
   if (left.getCubeRecur(phase ^ isNegEdge(), ith, target, res))
      return true;
   BddNode right = getRight();
   res = old & ~(_BddMgr->getSupport(getLevel()));
   if (right.getCubeRecur(phase ^ isNegEdge(), ith, target, res))
      return true;

   return false;
}

vector<BddNode>
BddNode::getAllCubes() const
{
   vector<BddNode> allCubes;
   BddNode cube = BddNode::_one;
   getAllCubesRecur(false, cube, allCubes);
   return allCubes;
}

void
BddNode::getAllCubesRecur
(bool phase, BddNode& cube, vector<BddNode>& allCubes) const
{
   if (isTerminal()) {
      if (!(phase ^ isNegEdge()))
         allCubes.push_back(cube);
      return;
   }

   BddNode old = cube;
   BddNode left = getLeft();
   cube = old & _BddMgr->getSupport(getLevel());
   left.getAllCubesRecur(phase ^ isNegEdge(), cube, allCubes);
   BddNode right = getRight();
   cube = old & ~(_BddMgr->getSupport(getLevel()));
   right.getAllCubesRecur(phase ^ isNegEdge(), cube, allCubes);
}

// Assume this BddNode is a cube
string
BddNode::toString() const
{
   string str;
   toStringRecur(false, str);
   return str;
}

// return true if a cube is found
bool
BddNode::toStringRecur(bool phase, string& str) const
{
   if (isTerminal())
      return (!(phase ^ isNegEdge()));

   stringstream sstr;
   if (getLeft().toStringRecur(phase ^ isNegEdge(), str)) {
      sstr << "(" << getLevel() << ") ";
      str += sstr.str();
      return true;
   }
   else if (getRight().toStringRecur(phase ^ isNegEdge(), str)) {
      sstr << "!(" << getLevel() << ") ";
      str += sstr.str();
      return true;
   }
   return false;
}
//
// [BONUS TODO]... ended

ostream&
operator << (ostream& os, const BddNode& n)
{
   size_t nNodes = 0;
   n.print(os, 0, nNodes);
   n.unsetVisitedRecur();
   os << endl << endl << "==> Total #BddNodes : " << nNodes << endl;
   return os;
}

void
BddNode::print(ostream& os, size_t indent, size_t& nNodes) const
{
   for (size_t i = 0; i < indent; ++i)
      os << ' ';
   BddNodeInt* n = getBddNodeInt();
   os << '[' << getLevel() << "](" << (isNegEdge()? '-' : '+') << ") ";
   if (_debugBddAddr)
      os << n << " ";
   if (_debugRefCount)
      os << "(" << n->getRefCount() << ")";
   if (n->isVisited()) {
      os << " (*)";
      return;
   }
   else ++nNodes;
   n->setVisited();
   if (!isTerminal()) {
      os << endl;
      n->getLeft().print(os, indent + 2, nNodes);
      os << endl;
      n->getRight().print(os, indent + 2, nNodes);
   }
}

void
BddNode::unsetVisitedRecur() const
{
   BddNodeInt* n = getBddNodeInt();
   if (!n->isVisited()) return;
   n->unsetVisited();
   if (!isTerminal()) {
      n->getLeft().unsetVisitedRecur();
      n->getRight().unsetVisitedRecur();
   }
}

void
BddNode::drawBdd(const string& name, ofstream& ofile) const
{
   // TODO
   ofile << "digraph {" << endl;
   ofile << "   node [shape = plaintext];" << endl;
   ofile << "   ";
   for (unsigned l = getLevel(); l > 0; --l)
      ofile << l << " -> ";
   ofile << "0 [style = invis];" << endl;
   ofile << "   { rank = source; \"" << name << "\"; }" << endl;
   ofile << "   node [shape = ellipse];" << endl;
   ofile << "   \"" << name << "\" -> \"" << getLabel()
         << "\" [color = blue]";
   ofile << (isNegEdge()? " [arrowhead = odot]" : ";") << endl;

   drawBddRecur(ofile);

   ofile << "   { rank = same; 0; \"One\"; }" << endl;
   ofile << "}" << endl;

   unsetVisitedRecur( );
}

void
BddNode::drawBddRecur(ofstream& ofile) const
{
   // TODO
   BddNodeInt* n = getBddNodeInt();
   if (n->isVisited())
      return;
   n->setVisited();
   if(isTerminal())
      return;
   BddNode left = getLeft();
   BddNode right = getRight();

   ofile << "   { rank = same; " << getLevel() << "; \"" << getLabel()
         << "\"; }\n";

   ofile << "   \"" << getLabel() << "\" -> \"" << left.getLabel() << "\"";
   ofile << ((left.isNegEdge())? " [arrowhead=odot]" : ";") << endl;

   ofile << "   \"" << getLabel() << "\" -> \"" << right.getLabel()
         << "\"[style = dotted ] [color=red]";
   ofile << ((right.isNegEdge())? " [arrowhead=odot]" : ";") << endl;

   left.drawBddRecur(ofile);
   right.drawBddRecur(ofile);
}

// Don't put this->isNegEdge() on label
string
BddNode::getLabel() const
{
   if (isTerminal())
      return "One";

   ostringstream str;
   str << getBddNodeInt();

   return str.str();
}

