#include <boost/python.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include "smt-switch/utils.h"

#include "framework/ts.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/symsim_branch.h"
#include "framework/state_simplify.h"

// CHECK url: 
//   https://cs.brown.edu/~jwicks/boost/libs/python/doc/tutorial/doc/html/python/object.html
//   https://www.qiniu.com/qfans/qnso-58096040
//   https://stackoverflow.com/questions/15842126/feeding-a-python-list-into-a-function-taking-in-a-vector-with-boost-python
//   https://wiki.python.org/moin/boost.python
//   https://py3c.readthedocs.io/en/latest/reference.html
//   

namespace wasim {
  /*
    * This is a wrapper class for all the exceptions
    * that will be thrown from the Python code.
    */
  struct PyWASIMException : public boost::exception
  {
      PyObject* exception;
      std::string message;

      PyWASIMException(PyObject* ex, const std::string& msg)
          : exception(ex)
          , message(msg)
      {}

      PyWASIMException(PyObject* ex, const char* msg)
          : exception(ex)
          , message(msg)
      {}

      virtual ~PyWASIMException() throw () {}
  };

  void translateWASIMException(PyWASIMException const& ex) {
    PyErr_SetString(ex.exception, ex.message.c_str());
  }

  boost::multiprecision::cpp_int pyint_to_cppint(const  boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      PyObject* pystr = PyObject_Str(pyobj);
      std::string lstr(PyUnicode_AsUTF8(pystr));
      Py_DECREF(pystr);

      return boost::multiprecision::cpp_int(lstr);
  }

  std::string pystr_to_string(const boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      PyObject* pystr = PyObject_Str(pyobj);
      std::string lstr(PyUnicode_AsUTF8(pystr));
      Py_DECREF(pystr);
      return lstr;
  }

  boost::python::object string_to_pyint(const std::string& s)
  {
      PyObject* oi = PyLong_FromString((char*)s.c_str(), NULL, 0);
      boost::python::object pi(boost::python::handle<>(boost::python::borrowed(oi)));
      return pi;
  }

  boost::python::object cppint_to_pyint(const boost::multiprecision::cpp_int& i)
  { 
      std::string si = i.str();
      return string_to_pyint(si);
  }


  bool is_py_int_or_long(const boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      return PyLong_Check(pyobj);
  }



// ----------------------------------------------------------------------


  struct NodeType;
  struct NodeRef;
  struct SolverRef;
  struct StateRef;
  struct TransSys;
  struct Symsimulator;
  struct Symsimbranch;


  struct SolverRef {
    SolverRef() { throw PyWASIMException(PyExc_RuntimeError, "Cannot create Solver directly. Use the context object."); }
    SolverRef(const smt::SmtSolver& ptr) : solver(ptr) { }
    SolverRef(const SolverRef & other) : solver(other.solver) {}

    void push() { solver->push(); }
    void pop() { solver->pop(); }
    void assert_formula(NodeRef * a);
    bool check_sat();
    bool check_sat_assuming(const boost::python::list & noderef_list);
    NodeRef * get_value(NodeRef * a) ;
    NodeType * make_bool();
    NodeType * make_bvsort(u_int64_t width);
    NodeRef * make_constant(boost::python::object c, NodeType * ntype) ;

    protected:
      friend class TransSys; // TransSys can use the same solver to build multiple transys
      // I suggest we do not rely on overloading in this case, just give another name
      friend NodeRef* expr_simplify_ite_wrap(const NodeRef*, const boost::python::list&, const SolverRef*);
      smt::SmtSolver solver;
  }; // end of SolverRef

  struct NodeType {

    NodeType()  { throw PyWASIMException(PyExc_RuntimeError, "Cannot create NodeType directly. Use the context object."); }
    NodeType(const smt::Sort& ptr) : sort(ptr) { }
    NodeType(const NodeType& nr) : sort(nr.sort) { }

    smt::SortKind sortkind() const {return sort->get_sort_kind(); }
    int bitwidth() const { return sort->get_width();  }
    int arity() const { return sort->get_arity(); }

    bool isBool() const { return sort->get_sort_kind() == smt::SortKind::BOOL; }
    bool isBitvector() const { return sort->get_sort_kind() == smt::SortKind::BV; }
    bool isArray() const { return sort->get_sort_kind() == smt::SortKind::ARRAY; }

    std::string to_string() const { return sort->to_string(); }
    size_t hash() const {return sort->hash();}

  protected:
    friend struct SolverRef;
    friend struct StateRef;
    friend struct TransSys;
    smt::Sort sort;
  };

  struct NodeRef {

    NodeRef() { throw PyWASIMException(PyExc_RuntimeError, "Cannot create Term directly. Use the context object."); }

    NodeRef(const smt::Term& ptr, const smt::SmtSolver & sptr) : 
      solver(sptr) , node(ptr) { }

    NodeRef(const NodeRef& nr) : solver(nr.solver), node(nr.node) { }

    ~NodeRef()  { }

    NodeRef operator=(const NodeRef& other)
    {
        if(this != &other) { 
            node = other.node;
            solver = other.solver;
        }
        return *this;
    }

    bool is_symbol() const { return node->is_symbol(); }
    bool is_value() const { return node->is_value(); }
    uint64_t to_int() const { return node->to_int(); }
    std::string to_string() const { return node->to_string(); }
    NodeType * getType() const { return new NodeType(node->get_sort()); }
    std::string to_smt2_fun(const std::string & n) const {
      smt::UnorderedTermSet free_vars;
      smt::get_free_symbols(node, free_vars);

      std::string args;
      bool first = true;
      for (const auto & v : free_vars) {
        if (!first)
          args += " ";
        args += ( "(" + v->to_string() + " " + v->get_sort()->to_string() + ")" );
        first = false;
      }
      args += ( ") " +  node->get_sort()->to_string() + " ");

      std::string ret = "(define-fun " + n + args + node->to_string() +")";
      return ret;
    } // end of to_smt2_fun

    smt::Op get_op() const { return node->get_op(); }
    
    NodeRef * substitute(const boost::python::dict & d) const { 
      smt::UnorderedTermMap subst;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);

          if(k.check() && v.check())
            subst.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in substitute");
      }
      return new NodeRef(solver->substitute(node, subst), solver);
    }

    boost::python::list args() const {
      boost::python::list l;
      for(const auto & t: *node) {
        l.append(new NodeRef(t, solver));
      }
      return l;
    }

    size_t hash() const { return node->hash();  }
    std::string short_str() const {
      if(node->is_symbol() || node->is_value()) {
        return node->to_string();
      }
      auto op = node->get_op();
      return "("+op.to_string()+" ..."+")";
    }

    boost::python::list get_vars() const {
      boost::python::list ret;
      smt::UnorderedTermSet free_var_set;
      smt::get_free_symbols(node, free_var_set);
      for (const auto & n : free_var_set) {
        ret.append(new NodeRef(n, solver));
      }
      return ret;
    }

    SolverRef * get_solver() const { return new SolverRef(solver); }

    NodeRef* python_copy() const { // will be used in `__copy__`
      return new NodeRef(*this);
    }

    NodeRef* complement() const
    {
        return _unOp(smt::PrimOp::Not, smt::PrimOp::BVNot, "complement bv/bool");
    }

    NodeRef* negate() const
    {
        return _unOp(smt::PrimOp::Negate, smt::PrimOp::BVNeg, "negate bv/bool");
    }

    NodeRef* logicalAnd(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::And, smt::PrimOp::BVAnd, "and bv/bool", other);
    }

    NodeRef* logicalAndInt(int r) const 
    {
        return _binOpL(smt::PrimOp::BVAnd, "bvand", r);
    }

    NodeRef* logicalAndRInt(int l) const
    {
        return _binOpR(smt::PrimOp::BVAnd, "bvand", l);
    }

    NodeRef* logicalOr(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::Or, smt::PrimOp::BVOr, "or bv/bool", other);
    }

    NodeRef* logicalOrInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVOr, "bvor", r);
    }

    NodeRef* logicalOrRInt(int l) const
    {
        return _binOpR(smt::PrimOp::BVOr, "bvor", l);
    }

    NodeRef* logicalXor(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::Xor, smt::PrimOp::BVXor, "xor bv/bool", other);
    }

    NodeRef* logicalXorInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVXor, "bvxor", r);
    }

    NodeRef* logicalXorRInt(int l) const
    {
        return _binOpR(smt::PrimOp::BVXor, "bvxor", l);
    }

    // add //
    NodeRef* add(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::BVAdd, "bvadd", other);
    }

    NodeRef* addInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVAdd, "bvadd", r);
    }

    NodeRef* raddInt(int r) const
    {
        return _binOpR(smt::PrimOp::BVAdd, "bvadd", r);
    }

    // sub //
    NodeRef* sub(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::BVSub, "bvsub", other);
    }

    NodeRef* subInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVSub, "bvsub", r);
    }

    NodeRef* rsubInt(int r) const
    {
        return _binOpR(smt::PrimOp::BVSub, "bvsub", r);
    }

    // udiv //
    NodeRef* udiv(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::BVUdiv, "bvudiv", other);
    }

    NodeRef* udivInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVUdiv, "bvudiv", r);
    }

    NodeRef* rudivInt(int r) const
    {
        return _binOpR(smt::PrimOp::BVUdiv, "bvudiv", r);
    }

    // urem //
    NodeRef* urem(NodeRef* other)
    {
        return _binOp(smt::PrimOp::BVUrem, "bvurem", other);
    }

    NodeRef* uremInt(int r)
    {
        return _binOpL(smt::PrimOp::BVUrem, "bvurem", r);
    }

    NodeRef* ruremInt(int l)
    {
        return _binOpR(smt::PrimOp::BVUrem, "bvurem", l);
    }

    // shl
    NodeRef* shl(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::BVShl, "bvshl", other);
    }

    NodeRef* shlInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVShl, "bvshl", r);
    }

    NodeRef* rshlInt(int r) const
    {
        return _binOpR(smt::PrimOp::BVShl, "bvshl", r);
    }

    // shr //
    NodeRef* lshr(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::BVLshr, "bvlshr", other);
    }

    NodeRef* lshrInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVLshr, "bvlshr", r);
    }

    NodeRef* rlshrInt(int r) const
    {
        return _binOpR(smt::PrimOp::BVLshr, "bvlshr", r);
    }

    // mul //
    NodeRef* mul(NodeRef* other) const
    {
        return _binOp(smt::PrimOp::BVMul, "bvmul", other);
    }

    NodeRef* mulInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVMul, "bvmul", r);
    }

    NodeRef* rmulInt(int r) const
    {
        return _binOpR(smt::PrimOp::BVMul, "bvmul", r);
    }

    // eq/neq //
    NodeRef* eq(NodeRef * other) const
    {
        return _binOp(smt::PrimOp::Equal, "equal", other);
    }

    NodeRef* eqInt(int r) const
    {
        return _binOpL(smt::PrimOp::Equal, "equal", r);
    }

    NodeRef* eqIntR(int l) const
    {
        return _binOpR(smt::PrimOp::Equal, "equal", l);
    }

    NodeRef* neq(NodeRef * other) const
    {
        return _binOp(smt::PrimOp::Distinct, "distinct", other);
    }

    NodeRef* neqInt(int r) const
    {
        return _binOpL(smt::PrimOp::Distinct, "distinct", r);
    }

    NodeRef* neqIntR(int l) const
    {
        return _binOpR(smt::PrimOp::Distinct, "distinct", l);
    }


    NodeRef* ult(NodeRef * other) const
    {
        return _binOp(smt::PrimOp::BVUlt, "bvult", other);
    }

    NodeRef* ultInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVUlt, "bvult", r);
    }

    NodeRef* ultIntR(int l) const
    {
        return _binOpR(smt::PrimOp::BVUlt, "bvult", l);
    }


    NodeRef* ugt(NodeRef * other) const
    {
        return _binOp(smt::PrimOp::BVUgt, "bvugt", other);
    }

    NodeRef* ugtInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVUgt, "bvugt", r);
    }

    NodeRef* ugtIntR(int l) const
    {
        return _binOpR(smt::PrimOp::BVUgt, "bvugt", l);
    }


    NodeRef* ule(NodeRef * other) const
    {
        return _binOp(smt::PrimOp::BVUle, "bvule", other);
    }

    NodeRef* uleInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVUle, "bvule", r);
    }

    NodeRef* uleIntR(int l) const
    {
        return _binOpR(smt::PrimOp::BVUle, "bvule", l);
    }


    NodeRef* uge(NodeRef * other) const
    {
        return _binOp(smt::PrimOp::BVUge, "bvuge", other);
    }

    NodeRef* ugeInt(int r) const
    {
        return _binOpL(smt::PrimOp::BVUge, "bvuge", r);
    }

    NodeRef* ugeIntR(int l) const
    {
        return _binOpR(smt::PrimOp::BVUge, "bvuge", l);
    }

    NodeRef* getItemInt(int idx) const
    {
        return Extract_operator(this, idx, idx);
    }
    
    NodeRef* slice(int hi, int lo) const 
    {
        return Extract_operator(this, hi, lo);
    }

    NodeRef* getitem_dispatch( boost::python::object key){
      if (boost::python::extract<int>(key).check()) {
        int idx = boost::python::extract<int>(key);
        return this->getItemInt(idx);
      }
      else if(boost::python::extract<boost::python::slice>(key).check()){
        boost::python::slice s = boost::python::extract<boost::python::slice>(key);
        int high = boost::python::extract<int>(s.start());
        int low = boost::python::extract<int>(s.stop());
        return this->slice(high, low);
      }
      else{
        throw PyWASIMException(PyExc_AttributeError, "Index must be [x] or [x:y]");
        return nullptr;
      }
    }

    NodeRef* read(NodeRef * addr) const 
    {
        return _binOp(smt::PrimOp::Select, "LOAD", addr);
    }


  protected:
    friend struct SolverRef;
    friend struct StateRef;
    friend struct TransSys;
    friend struct Symsimulator;
    friend struct Symsimbranch;
    friend NodeRef* expr_simplify_ite_wrap(const NodeRef*, const boost::python::list&, const SolverRef*);
    
    smt::SmtSolver solver;
    smt::Term node;

    unsigned bvwidth() const { 
      if ( node->get_sort()->get_sort_kind() != smt::SortKind::BV) return 0;
      return node->get_sort()->get_width(); }
    bool isbool() const { return node->get_sort()->get_sort_kind() == smt::SortKind::BOOL; }
    // ---------------------------------------------------------------------- //
    NodeRef* _unOp(smt::PrimOp opBool , smt::PrimOp opBv, const char* opName) const
    {   
        try{
          if (isbool())
            return new NodeRef(solver->make_term(opBool, node), solver);
          if (bvwidth())
            return new NodeRef(solver->make_term(opBv, node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        } 

        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    NodeRef* _binOp(
        smt::PrimOp boolOp, 
        smt::PrimOp bvOp, 
        const char* opName,
        NodeRef* other) const
    {
        try{
          if (isbool())
            return new NodeRef(solver->make_term(boolOp, node, other->node), solver);
          if (bvwidth())
            return new NodeRef(solver->make_term(bvOp, node, other->node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        } 
        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    NodeRef* _binOp(smt::PrimOp op, const char* opName, NodeRef* other) const
    {
        try{
          return new NodeRef(solver->make_term(op, node, other->node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    NodeRef* _binOpL(smt::PrimOp op, const char* opName, int r) const
    {
      auto sz = bvwidth();
      if(sz != 0) {
        try{
          auto cterm = solver->make_term(r, solver->make_sort(smt::SortKind::BV, sz));
          return new NodeRef(solver->make_term(op, node, cterm), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      } else if (isbool()) {
        try{
          auto cterm = solver->make_term((bool)(r));
          return new NodeRef(solver->make_term(op, node, cterm), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    NodeRef* _binOpR(smt::PrimOp op, const char* opName, int r) const
    {
      auto sz = bvwidth();
      if(sz != 0) {
        try{
          auto cterm = solver->make_term(r, solver->make_sort(smt::SortKind::BV, sz));
          return new NodeRef(solver->make_term(op, cterm, node ), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      } else if (isbool()) {
        try{
          auto cterm = solver->make_term((bool)(r));
          return new NodeRef(solver->make_term(op, cterm, node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    // static ones
    static NodeRef* triOp(smt::PrimOp Op, const char* opName,
                   NodeRef * t1, NodeRef * t2, NodeRef * t3)
    {   
      try{
        return new NodeRef(t1->solver->make_term(Op, t1->node, 
          t2->node, t3->node), t1->solver);
      } catch (SmtException e) {
        throw PyWASIMException(PyExc_TypeError, e.what());
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    static NodeRef * _s_binOp(smt::PrimOp op, const char* opName, NodeRef* l, NodeRef* r) {
        try{
          return new NodeRef(l->solver->make_term(op, l->node, r->node), l->solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    static NodeRef* _s_binOp(
        smt::PrimOp boolOp, 
        smt::PrimOp bvOp, 
        const char* opName,
        NodeRef* l, NodeRef* r) {
        try{
          if (l->isbool())
            return new NodeRef(l->solver->make_term(boolOp, l->node, r->node), l->solver);
          if (l->bvwidth())
            return new NodeRef(l->solver->make_term(bvOp, l->node, r->node), l->solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        } 
        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    static NodeRef* _s_binOpL(smt::PrimOp op, const char* opName, NodeRef* l, int r) {
      auto sz = l->bvwidth();
      if(sz != 0) {
        try{
          auto cterm = l->solver->make_term(r, l->solver->make_sort(smt::SortKind::BV, sz));
          return new NodeRef(l->solver->make_term(op, l->node, cterm), l->solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      } else if (l->isbool()) {
        try{
          auto cterm = l->solver->make_term((bool)(r));
          return new NodeRef(l->solver->make_term(op, l->node, cterm), l->solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    static NodeRef* _s_binOpR(smt::PrimOp op, const char* opName, int l, NodeRef* r) {
      auto sz = r->bvwidth();
      if(sz != 0) {
        try{
          auto cterm = r->solver->make_term(l, r->solver->make_sort(smt::SortKind::BV, sz));
          return new NodeRef(r->solver->make_term(op, cterm, r->node), r->solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      } else if (r->isbool()) {
        try{
          auto cterm = r->solver->make_term((bool)(l));
          return new NodeRef(r->solver->make_term(op, cterm, r->node), r->solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

  public:
    static bool is_sat(const boost::python::list & l) {
      if (len(l) == 0)
        throw PyWASIMException(PyExc_TypeError, std::string("expect non-empty list for is_sat "));
      smt::TermVec args;
      smt::SmtSolver slv;
      for (unsigned i=0; i != boost::python::len(l); i++ ) {
        boost::python::extract<NodeRef *> ni(l[i]);
        if(ni.check()) {
          if (i==0)
            slv = ni()->solver;
          args.push_back(ni()->node);
        } else {
            throw PyWASIMException(
                PyExc_TypeError, "Argument to is_sat must be terms.");
            return NULL;
        }
      }
      return slv->check_sat_assuming(args).is_sat();
    }

    static bool same_expr(NodeRef * a, NodeRef * b) {
      return a->node == b->node;
    }

    static NodeRef * make_term(smt::Op op, const boost::python::list & l) {
      if (len(l) == 0)
        throw PyWASIMException(PyExc_TypeError, std::string("expect non-empty args for make_term "));

      smt::TermVec args;
      smt::SmtSolver slv;
      for (unsigned i=0; i != boost::python::len(l); i++ ) {
        boost::python::extract<NodeRef *> ni(l[i]);
        if(ni.check()) {
          if (i==0)
            slv = ni()->solver;
          args.push_back(ni()->node);
        } else {
            throw PyWASIMException(
                PyExc_TypeError, "Argument to make_term must be terms.");
            return NULL;
        }
      }
      try {
        return new NodeRef(slv->make_term(op, args), slv);
      } catch(SmtException e) {
        throw PyWASIMException(PyExc_TypeError, e.what());
      }
    }

    static NodeRef * ITE_operator(NodeRef * cond, NodeRef * thenExpr, NodeRef * elseExpr) {
      return triOp(smt::PrimOp::Ite, "ITE", cond, thenExpr, elseExpr);
    }


    static NodeRef * Store_operator(NodeRef * array, NodeRef * addrExpr, NodeRef * dataExpr) {
      return triOp(smt::PrimOp::Store, "STORE", array, addrExpr, dataExpr);
    }

    static NodeRef * Load_operator(NodeRef * array, NodeRef * addrExpr) {
      return _s_binOp(smt::PrimOp::Select, "LOAD", array, addrExpr);
    }

    static NodeRef* Extract_operator(const NodeRef* obj, int beg, int end)
    {
      if (obj->bvwidth() == 0)
        throw PyWASIMException(PyExc_TypeError, "Incorrect type for SLICE" );
      smt::Op exop(smt::PrimOp::Extract, beg, end);
      try{
        return new NodeRef(obj->solver->make_term(exop, obj->node), obj->solver);
      } catch (SmtException e) {
        throw PyWASIMException(PyExc_TypeError, e.what());
      }
      throw PyWASIMException(PyExc_TypeError, "Incorrect type for SLICE" );
      return NULL;
    }


    static NodeRef* logicalXnor(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVXnor, "bvxnor", l, r);
    }

    static NodeRef* logicalNand(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVNand, "bvnand", l, r);
    }

    static NodeRef* logicalNor(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVNor, "bvnor", l, r);
    }

    // sdiv //
    static NodeRef* sdiv(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVSdiv, "bvsdiv", l, r);
    }

    static NodeRef* sdivInt(NodeRef* l, int r)
    {
        return _s_binOpL(smt::PrimOp::BVSdiv, "bvsdiv", l, r);
    }

    static NodeRef* rsdivInt(int l, NodeRef* r)
    {
        return _s_binOpR(smt::PrimOp::BVSdiv, "bvsdiv", l, r);
    }

    // smod //
    static NodeRef* smod(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVSmod, "bvsmod", l, r);
    }

    static NodeRef* smodInt(NodeRef* l, int r)
    {
        return _s_binOpL(smt::PrimOp::BVSmod, "bvsmod", l, r);
    }

    static NodeRef* rsmodInt(int l, NodeRef* r)
    {
        return _s_binOpR(smt::PrimOp::BVSmod, "bvsmod", l, r);
    }

    // srem //
    static NodeRef* srem(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVSrem, "bvsrem", l, r);
    }

    static NodeRef* sremInt(NodeRef* l, int r)
    {
        return _s_binOpL(smt::PrimOp::BVSrem, "bvsrem", l, r);
    }

    static NodeRef* rsremInt(int l, NodeRef* r)
    {
        return _s_binOpR(smt::PrimOp::BVSrem, "bvsrem", l, r);
    }

    static NodeRef* ashr(NodeRef* l, NodeRef* r)
    {
        return _s_binOp(smt::PrimOp::BVAshr, "bvashr", l, r);
    }

    static NodeRef* ashrInt(NodeRef* l, int r)
    {
        return _s_binOpL(smt::PrimOp::BVAshr, "bvashr", l, r);
    }

    static NodeRef* rashrInt(int l, NodeRef* r)
    {
        return _s_binOpR(smt::PrimOp::BVAshr, "bvashr", l, r);
    }

    static NodeRef* concat(NodeRef* hi, NodeRef* lo)
    {
        return _s_binOp(smt::PrimOp::Concat, "concat", hi, lo);
    }

    static NodeRef* concatList(const boost::python::list& l)
    {
      // number of arguments.
      if (boost::python::len(l) < 1) {
          throw PyWASIMException(
              PyExc_RuntimeError, "Cannot concat empty list");
          return NULL;
      }

      smt::TermVec args;
      smt::SmtSolver slv;
      for (unsigned i=0; i != boost::python::len(l); i++ ) {
        boost::python::extract<NodeRef *> ni(l[i]);
        if(ni.check()) {
          if (i==0)
            slv = ni()->solver;
          args.push_back(ni()->node);
        } else {
            throw PyWASIMException(
                PyExc_TypeError, "Argument to concat must be terms.");
            return NULL;
        }
      }
      if (args.size() == 1)
        return new NodeRef(args[0], slv);
      try {
        return new NodeRef(slv->make_term(smt::PrimOp::Concat, args), slv);
      } catch(SmtException e) {
        throw PyWASIMException(PyExc_TypeError, e.what());
      }
      return NULL;
    } // end of concatList

    static NodeRef* lrotate(NodeRef* obj, int par)
    {
      smt::Op op(smt::PrimOp::Rotate_Left, par);
      try {
        return new NodeRef(obj->solver->make_term(op, obj->node), obj->solver);
      } catch(SmtException e) { throw PyWASIMException(PyExc_TypeError, e.what()); }
      return NULL;
    }

    static NodeRef* rrotate(NodeRef* obj, int par)
    {
      smt::Op op(smt::PrimOp::Rotate_Right, par);
      try {
        return new NodeRef(obj->solver->make_term(op, obj->node), obj->solver);
      } catch(SmtException e) { throw PyWASIMException(PyExc_TypeError, e.what()); }
      return NULL;
    }

    static NodeRef* zero_extend(NodeRef* obj, int morewidth)
    {
      smt::Op op(smt::PrimOp::Zero_Extend, morewidth);
      try {
        return new NodeRef(obj->solver->make_term(op, obj->node), obj->solver);
      } catch(SmtException e) { throw PyWASIMException(PyExc_TypeError, e.what()); }
      return NULL;
    }

    static NodeRef* sign_extend(NodeRef* obj, int morewidth)
    {
      smt::Op op(smt::PrimOp::Sign_Extend, morewidth);
      try {
        return new NodeRef(obj->solver->make_term(op, obj->node), obj->solver);
      } catch(SmtException e) { throw PyWASIMException(PyExc_TypeError, e.what()); }
      return NULL;
    }

    static NodeRef* slt(NodeRef * l, NodeRef * r) 
    {
        return _s_binOp(smt::PrimOp::BVSlt, "bvslt", l, r);
    }

    static NodeRef* sgt(NodeRef * l, NodeRef * r)
    {
        return _s_binOp(smt::PrimOp::BVSgt, "bvsgt", l, r);
    }

    static NodeRef* sle(NodeRef * l, NodeRef * r)
    {
        return _s_binOp(smt::PrimOp::BVSle, "bvsle", l, r);
    }

    static NodeRef* sge(NodeRef * l, NodeRef * r)
    {
        return _s_binOp(smt::PrimOp::BVSge, "bvsge", l, r);
    }

    static NodeRef* sltInt(NodeRef* l, int r) 
    {
        return _s_binOpL(smt::PrimOp::BVSlt, "bvslt", l, r);
    }

    static NodeRef* sgtInt(NodeRef* l, int r) 
    {
        return _s_binOpL(smt::PrimOp::BVSgt, "bvsgt", l, r);
    }

    static NodeRef* sleInt(NodeRef* l, int r) 
    {
        return _s_binOpL(smt::PrimOp::BVSle, "bvsle", l, r);
    }

    static NodeRef* sgeInt(NodeRef* l, int r) 
    {
        return _s_binOpL(smt::PrimOp::BVSge, "bvsge", l, r);
    }

    static NodeRef* sltIntR(int l, NodeRef* r) 
    {
        return _s_binOpR(smt::PrimOp::BVSlt, "bvslt", l, r);
    }

    static NodeRef* sgtIntR(int l, NodeRef* r) 
    {
        return _s_binOpR(smt::PrimOp::BVSgt, "bvsgt", l, r);
    }

    static NodeRef* sleIntR(int l, NodeRef* r) 
    {
        return _s_binOpR(smt::PrimOp::BVSle, "bvsle", l, r);
    }

    static NodeRef* sgeIntR(int l, NodeRef* r) 
    {
        return _s_binOpR(smt::PrimOp::BVSge, "bvsge", l, r);
    }

  }; // end of NodeRef


  /* class State */
  struct StateRef {
    StateRef() { throw PyWASIMException(PyExc_RuntimeError, "Cannot create State directly. Use the context object."); }
    StateRef(const StateRef & other) : solver(other.solver ), sptr(other.sptr) { }
    StateRef(StateAsmpt * ptr, smt::SmtSolver osolver) : solver(osolver), sptr(ptr) { }
    StateRef(const smt::UnorderedTermMap & sv,
             const smt::TermVec & asmpt,
             const std::vector<std::string> & assumption_interp, smt::SmtSolver osolver) :
        solver(osolver), sptr(std::make_shared<StateAsmpt>( sv,asmpt, assumption_interp )) {}

    StateRef * clone() const {
      return new StateRef(new StateAsmpt(*sptr), solver);
    }

    // test the type of the return list and see how that works
    boost::python::list get_assumptions() const {
      boost::python::list ret;
      for (const auto & a : sptr->get_assumptions())
        ret.append( new NodeRef(a, solver) ); // how this is managed?
      return ret;
    }

    // NodeRef* get_assumption(size_t idx) const { /*TODO*/ } // can we return a list of NodeRef?
    void set_assumptions(const boost::python::list & l) {
      smt::TermVec newvec;
      for (size_t idx = 0; idx < boost::python::len(l); ++idx) {
        boost::python::extract<NodeRef *> ni(l[idx]);
        if(!ni.check()) {
          throw PyWASIMException(PyExc_RuntimeError, "set_assumptions requires a list of terms");
          return;
        }
        newvec.push_back(ni()->node);
      }
      sptr->update_assumptions().swap(newvec);
    }

    boost::python::list get_assumption_interps() const {
      boost::python::list ret;
      for (const auto & a : sptr->get_assumption_interpretations())
        ret.append( a ); // how this is managed?
      return ret;
    }

    void set_assumption_interps(const boost::python::list & l) {
      std::vector<std::string> newvec;
      for (size_t idx = 0; idx < boost::python::len(l); ++idx) {
        boost::python::extract<std::string> ni(l[idx]);
        if(!ni.check()) {
          throw PyWASIMException(PyExc_RuntimeError, "set_assumptions requires a list of strings");
          return;
        }
        newvec.push_back(ni());
      }
      sptr->update_assumption_interpretations().swap(newvec);
    }

    boost::python::list get_vars() const {
      boost::python::list ret;
      for (const auto & sv : sptr->get_sv()) {
        ret.append(new NodeRef(sv.first, solver));
      }
      return ret;
    }
    boost::python::list get_varnames() const {
      boost::python::list ret;
      for (const auto & sv : sptr->get_sv()) {
        ret.append(sv.first->to_string());
      }
      return ret;
    }

    boost::python::dict get_sv() const {
      boost::python::dict ret;
      for (const auto & sv : sptr->get_sv()) {
        NodeRef * k = new NodeRef(sv.first, solver);
        NodeRef * v = new NodeRef(sv.second, solver);
        ret[k] = v;
      }
      return ret;
    }

    NodeRef* get_term(const std::string & name) const  {
      for (const auto & sv : sptr->get_sv()) {
        if (name == sv.first->to_string())
          return new NodeRef(sv.second, solver);
      }
      throw PyWASIMException(PyExc_RuntimeError, "no statevar named" + name);
      return NULL;
    }

    void set_sv(const boost::python::dict & sv) {
      smt::UnorderedTermMap newmap;
      boost::python::list items = sv.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);

          if(k.check() && v.check())
            newmap.emplace(k()->node, v()->node);
      }
      sptr->update_sv().swap(newmap);
    }
    
    friend struct Symsimulator;
    friend struct Symsimbranch;

    protected:
      smt::SmtSolver solver;
      std::shared_ptr<StateAsmpt> sptr;
  }; // end of state

  void SolverRef::assert_formula(NodeRef * a) { solver->assert_formula(a->node); }
  bool SolverRef::check_sat() { return solver->check_sat().is_sat(); }
  bool SolverRef::check_sat_assuming(const boost::python::list & noderef_list) {
    smt::TermVec asmpts;
    for (ssize_t i = 0; i < len(noderef_list); ++ i) {
      boost::python::extract<NodeRef *> k(noderef_list[i]);
      if(k.check())
        asmpts.push_back(k()->node);
      else
        throw PyWASIMException(PyExc_RuntimeError, "Expecting NodeRef List in check_sat_assuming");
    }
    return solver->check_sat_assuming(asmpts).is_sat();
  }

  NodeRef * SolverRef::get_value(NodeRef * a) { return new NodeRef(solver->get_value(a->node), solver ); }

  NodeType * SolverRef::make_bool() { return new NodeType(solver->make_sort(smt::SortKind::BOOL)); }
  NodeType * SolverRef::make_bvsort(u_int64_t width) { return new NodeType(solver->make_sort(smt::SortKind::BV, width)); }
  NodeRef * SolverRef::make_constant(boost::python::object c, NodeType * ntype) {
    auto s = pystr_to_string(c);
    try{
      return new NodeRef(solver->make_term(s, ntype->sort),  solver);
    } catch(SmtException e) {
      throw PyWASIMException(PyExc_TypeError, e.what());
    }
    return NULL;
  }

  /* TransSys : ts */
  struct TransSys {

    TransSys(const std::string & btorname, const std::string & prefix = "") { 
      smt::SmtSolver solver = smt::BoolectorSolverFactory::create(false);
      solver->set_logic("QF_UFBV");
      solver->set_opt("incremental", "true");
      solver->set_opt("produce-models", "true");
      solver->set_opt("produce-unsat-assumptions", "true");
      sptr = std::make_shared<TransitionSystem> (solver);
      BTOR2Encoder btor_parser(btorname, *sptr, prefix);
    }

    TransSys(const std::string & btorname, SolverRef * slv, const std::string & prefix = "") { 
      sptr = std::make_shared<TransitionSystem> (slv->solver);
      BTOR2Encoder btor_parser(btorname, *sptr, prefix);
    }

    NodeRef * curr(NodeRef * term) const {
      return new NodeRef(sptr->curr(term->node), sptr->get_solver());
    }

    NodeRef * next(NodeRef * term) const {
      return new NodeRef(sptr->next(term->node), sptr->get_solver());
    }

    bool is_curr_var(NodeRef * sv) const {
      return sptr->is_curr_var(sv->node);
    }

    bool is_next_var(NodeRef * sv) const {
      return sptr->is_next_var(sv->node);
    }

    bool is_input_var(NodeRef * sv) const {
      return sptr->is_input_var(sv->node);
    }

    std::string get_name(NodeRef * t) const {
      return sptr->get_name(t->node);
    }

    NodeRef * lookup(const std::string & name) const {
      return new NodeRef(sptr->lookup(name), sptr->get_solver());
    }

    /* Gets a reference to the solver */
    SolverRef * get_solver() { 
      return new SolverRef(sptr->get_solver());
    }

    boost::python::list statevars() const { 
      boost::python::list ret;
      for(const auto & sv : sptr->statevars()) {
        ret.append(new NodeRef(sv, sptr->get_solver()));
      }
      return ret;
    }

    boost::python::list inputvars() const { 
      boost::python::list ret;
      for(const auto & sv : sptr->inputvars()) {
        ret.append(new NodeRef(sv, sptr->get_solver()));
      }
      return ret;
    }

    boost::python::list output_terms() const { 
      boost::python::list ret;
      for(const auto & sv : sptr->output_terms()) {
        ret.append(new NodeRef(sv, sptr->get_solver()));
      }
      return ret;
    }

    boost::python::dict init_constants() const {
      boost::python::dict ret;
      for (const auto & sv_update : sptr->init_constants()) {
        NodeRef * k = new NodeRef(sv_update.first, sptr->get_solver());
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    /* Returns the initial state constraints
    * @return a boolean term constraining the initial state
    */
    NodeRef * init() const { 
      return new NodeRef(sptr->init(), sptr->get_solver());
    }

    /* Returns the transition relation
    * @return a boolean term representing the transition relation
    */
    NodeRef * trans() const { 
      return new NodeRef(sptr->trans(), sptr->get_solver());
    }


    /* Returns the next state updates
    * @return a map of functional next state updates
    */
    boost::python::dict state_updates() const
    {
      boost::python::dict ret;
      for (const auto & sv_update : sptr->state_updates()) {
        NodeRef * k = new NodeRef(sv_update.first, sptr->get_solver());
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    /* @return the named terms mapping */
    boost::python::dict named_terms() const
    {
      boost::python::dict ret;
      for (const auto & sv_update : sptr->named_terms()) {
        std::string k = sv_update.first;
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    /** @return the constraints of the system
     */
    boost::python::list constraints() const
    {
      boost::python::list ret;
      for(const auto & c : sptr->constraints()) {
        ret.append(new NodeRef(c.first, sptr->get_solver()));
      }
      return ret;
    }

    boost::python::list prop() const
    {
      boost::python::list ret;
      for(const auto & c : sptr->prop()) {
        ret.append(new NodeRef(c, sptr->get_solver()));
      }
      return ret;
    }

    /** Whether the transition system is functional
     *  NOTE: This does *not* actually analyze the transition relation
     *  TODO possibly rename to be less confusing
     *  currently means state updates are always
     *    next_var := f(current_vars, inputs)
     *  however, it allows (certain) constraints still
     *  and does not require that every state has an update
     */
    bool is_functional() const { return sptr->is_functional(); }

    /** Whether the system is deterministic
     * this is a stronger condition than functional
     * TODO possibly rename to be less confusing
     *
     * deterministic (currently) means
     *   1) is_functional() is true
     *   2) every state has a next state for any input
     *      (i.e. no extra constraints)
     *   3) every state variable has an update function
     *       --> there exists exactly one next state
     *           if current vars and inputs are fixed
     */
      bool is_deterministic() const { return sptr->is_deterministic(); }

      /* Returns true iff all the symbols in the formula are current states */
      bool only_curr(NodeRef * term) const {
        return sptr->only_curr(term->node);
      }

      /* Returns true iff all the symbols in the formula are inputs and current
      * states */
      bool no_next(NodeRef * term) const {
        return sptr->no_next(term->node);
      }


    TransSys * clone () const {
      // make a copy of all lists etc
      return new TransSys(new TransitionSystem(*sptr));
    }

      friend struct Symsimulator;
      friend struct Symsimbranch;
    protected:
      std::shared_ptr<TransitionSystem> sptr;

      TransSys(TransitionSystem * ts) : sptr(ts) { }
  }; // struct TransSys

  /* TODO : simulator */
  struct Symsimulator {

    Symsimulator(TransSys * ts) {
      sptr = std::make_shared<SymbolicSimulator>(*(ts->sptr), ts->sptr->get_solver());
    }

    /// get the length of the trace
    unsigned tracelen() const { return sptr->tracelen(); }
    /// collect all assumptions on each frame
    boost::python::list all_assumptions() const {
      boost::python::list ret;
      for (const auto & a : sptr->all_assumptions())
        ret.append(new NodeRef(a, sptr->get_solver()));
      return ret;
    }
    /// collect all interpretations of assumptions on each frame
    boost::python::list all_assumption_interp() const {
      boost::python::list ret;
      for (const auto & s : sptr->all_assumption_interp())
        ret.append(s);
      return ret;
    }
    /// get the term for a variable
    NodeRef *var(const std::string & n) const {
      return new NodeRef(sptr->var(n), sptr->get_solver());
    }
    /// get the term for name n, then use the current symbolic
    /// variable assignment to substitute all variables in it
    /// if it contains input variable, use the most recent input
    /// variable assignment as well
    NodeRef * cur(const std::string & n) const {
      return new NodeRef(sptr->cur(n), sptr->get_solver());
    }
    /// print the current state variable assignment
    void print_current_step() const { sptr->print_current_step(); }
    /// get the assumptions (collected from all previous steps)
    void print_current_step_assumptions() const {sptr->print_current_step_assumptions();}

    boost::python::dict create_input_Xvars(const std::string & n) {
      auto ret_smt = sptr->create_input_Xvars(n);
      boost::python::dict ret;
      for (const auto & sv_update : ret_smt) {
        NodeRef * k = new NodeRef(sv_update.first, sptr->get_solver());
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    /// a shortcut to create symbolic variables/concrete values in a map
    boost::python::dict convert(const boost::python::dict & d) const {
      assignment_type vdict;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<std::string> k(key);
          boost::python::extract<int> v(value);
          boost::python::extract<std::string> v_str(value);
          boost::python::extract<NodeRef *> v_node(value);

          if (!k.check())
            throw PyWASIMException(PyExc_RuntimeError, "Expecting str->int/str/NodeRef map in convert");

          if (v.check())
            vdict.emplace(k(), v());
          else if (v_str.check())
            vdict.emplace(k(), v_str());
          else if (v_node.check())
            vdict.emplace(k(), v_node()->node );
          else
              throw PyWASIMException(PyExc_RuntimeError, "Expecting str->int/str/NodeRef map in convert");
      }
      auto ret_smt = sptr->convert(vdict);
      boost::python::dict ret;
      for (const auto & sv_update : ret_smt) {
        NodeRef * k = new NodeRef(sv_update.first, sptr->get_solver());
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    /// goto the previous simulation step
    void backtrack() { sptr->backtrack(); }

    void dump_waveform(const std::string & file_name,
                       const boost::python::dict & d, bool dump_all) { 
      // convert d to smt::UnorderedTermMap
      smt::UnorderedTermMap ivdict;

      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k_node(key);
          boost::python::extract<NodeRef *> v_node(value);

          if (!k_node.check() || !v_node.check())
            throw PyWASIMException(PyExc_RuntimeError, "Expecting NodeRef->NodeRef map in dump_waveform");
          ivdict.emplace(k_node()->node, v_node()->node);          
      }
      sptr->dump_waveform(file_name, ivdict, dump_all); 
    }

    /// use the given variable assignment to initialize
    void init(const boost::python::dict & d) {
      smt::UnorderedTermMap var_assignment;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);
          if(k.check() && v.check())
            var_assignment.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in init");
      }
      sptr->init(var_assignment);
    }

    void free_init(const boost::python::dict & d) {
      smt::UnorderedTermMap var_assignment;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);
          if(k.check() && v.check())
            var_assignment.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in init");
      }
      sptr->free_init(var_assignment);
    }


    /// re-assign the current state
    void set_current_state(const StateRef * s) { sptr->set_current_state(*(s->sptr.get())); }
    /// set the input variable values before simulating next step
    ///  (and also set some assumptions before the next step)
    void set_input(const boost::python::dict & iv, const boost::python::list & asmpts) {
      smt::UnorderedTermMap invar_assign;
      smt::TermVec pre_assumptions;

      boost::python::list items = iv.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);

          if(k.check() && v.check())
            invar_assign.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in set_input");
      }
      for (ssize_t i = 0; i < len(asmpts); ++i) {
        boost::python::extract<NodeRef *> key(asmpts[i]);
        if (key.check())
          pre_assumptions.push_back(key()->node);
        else
          throw PyWASIMException(PyExc_RuntimeError, "Expecting list of NodeRef in set_input");
      }
      sptr->set_input(invar_assign, pre_assumptions);
    }
    /// undo the input setting
    /// usage: set_input -> sim_one_step --> (a new state) -> backtrack ->
    /// undo_set_input
    void undo_set_input() { sptr->undo_set_input(); }

    /// similar to cur(), but will check no reference to the input variables
    NodeRef * interpret_state_expr_on_curr_frame(NodeRef * expr) const {
      return new NodeRef(sptr->interpret_state_expr_on_curr_frame(expr->node), sptr->get_solver());
    }

    NodeRef * interpret_input_and_state_expr_on_curr_frame(NodeRef * expr, const boost::python::dict & iv_term_dict) const {
      smt::UnorderedTermMap iv_map;
      boost::python::list items = iv_term_dict.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);

          if(k.check() && v.check())
            iv_map.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in interpret_input_and_state_expr_on_curr_frame");
      }
      return new NodeRef(sptr->interpret_input_and_state_expr_on_curr_frame(expr->node, iv_map), sptr->get_solver());
    }

    /// do simulation
    void sim_one_step() { sptr->sim_one_step(); }

    /// get the set of all X variables
    boost::python::list  get_Xs() const { 
      boost::python::list ret;
      for (const auto & x : sptr->get_Xs())
        ret.append(new NodeRef(x, sptr->get_solver()));
      return ret; 
    }

    /// get (a copy of) the current state
    StateRef * get_curr_state(const boost::python::list & assumptions)  {
      smt::TermVec assumpts;
      for(ssize_t i = 0; i < len(assumptions); ++i)  {
        boost::python::extract<NodeRef *>  aspt(assumptions[i]);
        if(aspt.check())
          assumpts.push_back(aspt()->node);
        else
          throw PyWASIMException(PyExc_RuntimeError, "Expecting list of NodeRef in get_curr_state");
      }
      StateAsmpt ret_state = sptr->get_curr_state(assumpts);
      return new StateRef( ret_state.get_sv(), ret_state.get_assumptions(), ret_state.get_assumption_interpretations(), sptr->get_solver() );
    }

    /// a shortcut to create a variable
    NodeRef * set_var(int bitwdth, const std::string & vname) {
      try {
        return new NodeRef(sptr->set_var(bitwdth, vname), sptr->get_solver());
      } catch(...) {
        throw PyWASIMException(PyExc_RuntimeError, "Variable name `" + vname + "` already exists");
      }
    }

    NodeRef * get_var(const std::string &vname) {
      try {
        auto t = sptr->get_solver()->get_symbol(vname);
        return new NodeRef(t, sptr->get_solver());
      } catch(...) {
        throw PyWASIMException(PyExc_RuntimeError, "Variable name `" + vname + "` does not exists");
      }
    }


    /// get solver
    SolverRef * get_solver() const { return new SolverRef(sptr->get_solver()); }

    protected:
      std::shared_ptr<SymbolicSimulator> sptr;
  };

  /* TODO : simulator */
  struct Symsimbranch {

    Symsimbranch(TransSys * ts) {
      sptr = std::make_shared<SymbolicSimulatorBranch>(*(ts->sptr), ts->sptr->get_solver());
    }

    size_t fork_branch(size_t branch_idx){
      return sptr-> fork_branch(branch_idx);
    }
    void finish_branch(size_t branch_idx){
      sptr-> finish_branch(branch_idx);
    }

    void add_assumption_interpreted(size_t branch_idx, NodeRef * asmpt, const std::string & interp) {
      sptr->add_assumption_interpreted(branch_idx, asmpt->node, interp);
    }

    /// get the length of the trace
    unsigned tracelen(size_t idx) const { return sptr->tracelen(idx); }
    /// collect all assumptions on each frame
    boost::python::list all_assumptions(size_t branch_idx) const {
      boost::python::list ret;
      for (const auto & a : sptr->all_assumptions(branch_idx))
        ret.append(new NodeRef(a, sptr->get_solver()));
      return ret;
    }
    /// collect all interpretations of assumptions on each frame
    boost::python::list all_assumption_interp(size_t branch_idx) const {
      boost::python::list ret;
      for (const auto & s : sptr->all_assumption_interp(branch_idx))
        ret.append(s);
      return ret;
    }
    /// get the term for a variable
    NodeRef *var(const std::string & n) const {
      return new NodeRef(sptr->var(n), sptr->get_solver());
    }
    /// get the term for name n, then use the current symbolic
    /// variable assignment to substitute all variables in it
    /// if it contains input variable, use the most recent input
    /// variable assignment as well
    NodeRef * cur(const std::string & n, size_t branch_idx) const {
      return new NodeRef(sptr->cur(n, branch_idx), sptr->get_solver());
    }
    /// print the current state variable assignment
    void print_current_step(size_t idx) const { sptr->print_current_step(idx); }
    /// get the assumptions (collected from all previous steps)
    void print_current_step_assumptions(size_t idx) const {sptr->print_current_step_assumptions(idx);}
    /// print the current state variable assignment
    void print_current_step_all_branches() const { sptr->print_current_step_all_branches(); }
    /// get the assumptions (collected from all previous steps)
    void print_current_step_assumptions_all_branches() const {sptr->print_current_step_assumptions_all_branches();}

    boost::python::dict create_input_Xvars(const std::string & n) {
      auto ret_smt = sptr->create_input_Xvars(n);
      boost::python::dict ret;
      for (const auto & sv_update : ret_smt) {
        NodeRef * k = new NodeRef(sv_update.first, sptr->get_solver());
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    bool is_Xvar(NodeRef * t) {
      return sptr->is_Xvar(t->node);
    }

    /// a shortcut to create symbolic variables/concrete values in a map
    boost::python::dict convert(const boost::python::dict & d) const {
      assignment_type vdict;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<std::string> k(key);
          boost::python::extract<int> v(value);
          boost::python::extract<std::string> v_str(value);
          boost::python::extract<NodeRef *> v_node(value);

          if (!k.check())
            throw PyWASIMException(PyExc_RuntimeError, "Expecting str->int/str/NodeRef map in convert");

          if (v.check())
            vdict.emplace(k(), v());
          else if (v_str.check())
            vdict.emplace(k(), v_str());
          else if (v_node.check())
            vdict.emplace(k(), v_node()->node );
          else
              throw PyWASIMException(PyExc_RuntimeError, "Expecting str->int/str/NodeRef map in convert");
      }
      auto ret_smt = sptr->convert(vdict);
      boost::python::dict ret;
      for (const auto & sv_update : ret_smt) {
        NodeRef * k = new NodeRef(sv_update.first, sptr->get_solver());
        NodeRef * v = new NodeRef(sv_update.second, sptr->get_solver());
        ret[k] = v;
      }
      return ret;
    }

    /// goto the previous simulation step
    void backtrack(size_t branch_idx) { sptr->backtrack(branch_idx); }

    /// dump waveform to a give file

    void dump_waveform(const std::string & file_name,
                       const boost::python::dict & d, bool dump_all, size_t branch_idx) { 
      // convert d to smt::UnorderedTermMap
      smt::UnorderedTermMap ivdict;

      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k_node(key);
          boost::python::extract<NodeRef *> v_node(value);

          if (!k_node.check() || !v_node.check())
            throw PyWASIMException(PyExc_RuntimeError, "Expecting NodeRef->NodeRef map in dump_waveform");
          ivdict.emplace(k_node()->node, v_node()->node);          
      }
      sptr->dump_waveform(file_name, ivdict, dump_all, branch_idx); 
    }

    /// use the given variable assignment to initialize
    void init(const boost::python::dict & d) {
      smt::UnorderedTermMap var_assignment;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);
          if(k.check() && v.check())
            var_assignment.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in init");
      }
      sptr->init(var_assignment);
    }

    void free_init(const boost::python::dict & d) {
      smt::UnorderedTermMap var_assignment;
      boost::python::list items = d.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);
          if(k.check() && v.check())
            var_assignment.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in init");
      }
      sptr->free_init(var_assignment);
    }


    /// re-assign the current state
    void set_current_state(const StateRef * s, size_t branch_idx) { sptr->set_current_state(*(s->sptr.get()), branch_idx); }
    /// set the input variable values before simulating next step
    ///  (and also set some assumptions before the next step)
    void set_input(const boost::python::dict & iv, const boost::python::list & asmpts, size_t branch_idx) {
      smt::UnorderedTermMap invar_assign;
      smt::TermVec pre_assumptions;

      boost::python::list items = iv.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);

          if(k.check() && v.check())
            invar_assign.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in set_input");
      }
      for (ssize_t i = 0; i < len(asmpts); ++i) {
        boost::python::extract<NodeRef *> key(asmpts[i]);
        if (key.check())
          pre_assumptions.push_back(key()->node);
        else
          throw PyWASIMException(PyExc_RuntimeError, "Expecting list of NodeRef in set_input");
      }
      sptr->set_input(invar_assign, pre_assumptions, branch_idx);
    }
    /// undo the input setting
    /// usage: set_input -> sim_one_step --> (a new state) -> backtrack ->
    /// undo_set_input
    void undo_set_input(size_t branch_idx) { sptr->undo_set_input(branch_idx); }

    /// similar to cur(), but will check no reference to the input variables
    NodeRef * interpret_state_expr_on_curr_frame(NodeRef * expr, size_t branch_idx) const {
      return new NodeRef(sptr->interpret_state_expr_on_curr_frame(expr->node, branch_idx), sptr->get_solver());
    }

    NodeRef * interpret_input_and_state_expr_on_curr_frame(NodeRef * expr, const boost::python::dict & iv_term_dict, size_t branch_idx) const {
      smt::UnorderedTermMap iv_map;
      boost::python::list items = iv_term_dict.items();
      for(ssize_t i = 0; i < len(items); ++i) {
          boost::python::object key = items[i][0];
          boost::python::object value = items[i][1];
          boost::python::extract<NodeRef *> k(key);
          boost::python::extract<NodeRef *> v(value);

          if(k.check() && v.check())
            iv_map.emplace(k()->node, v()->node);
          else
            throw PyWASIMException(PyExc_RuntimeError, "Expecting Term->Term map in interpret_input_and_state_expr_on_curr_frame");
      }
      return new NodeRef(sptr->interpret_input_and_state_expr_on_curr_frame(expr->node, iv_map, branch_idx), sptr->get_solver());
    }

    /// do simulation
    void sim_one_step(size_t branch_idx) { sptr->sim_one_step(branch_idx); }

    /// get the set of all X variables
    boost::python::list  get_Xs() const { 
      boost::python::list ret;
      for (const auto & x : sptr->get_Xs())
        ret.append(new NodeRef(x, sptr->get_solver()));
      return ret; 
    }

    /// get (a copy of) the current state
    StateRef * get_curr_state(const boost::python::list & assumptions, size_t branch_idx)  {
      smt::TermVec assumpts;
      for(ssize_t i = 0; i < len(assumptions); ++i)  {
        boost::python::extract<NodeRef *>  aspt(assumptions[i]);
        if(aspt.check())
          assumpts.push_back(aspt()->node);
        else
          throw PyWASIMException(PyExc_RuntimeError, "Expecting list of NodeRef in get_curr_state");
      }
      StateAsmpt ret_state = sptr->get_curr_state(assumpts, branch_idx);
      return new StateRef( ret_state.get_sv(), ret_state.get_assumptions(), ret_state.get_assumption_interpretations(), sptr->get_solver() );
    }

    /// a shortcut to create a variable
    NodeRef * set_var(int bitwdth, const std::string & vname) {
      try {
        return new NodeRef(sptr->set_var(bitwdth, vname), sptr->get_solver());
      } catch(...) {
        throw PyWASIMException(PyExc_RuntimeError, "Variable name `" + vname + "` already exists");
      }
    }

    NodeRef * get_var(const std::string &vname) {
      try {
        auto t = sptr->get_solver()->get_symbol(vname);
        return new NodeRef(t, sptr->get_solver());
      } catch(...) {
        throw PyWASIMException(PyExc_RuntimeError, "Variable name `" + vname + "` does not exists");
      }
    }

    /// get solver
    SolverRef * get_solver() const { return new SolverRef(sptr->get_solver()); }

    protected:
      std::shared_ptr<SymbolicSimulatorBranch> sptr;
  };
  
  /* state_simplify */
  NodeRef * expr_simplify_ite_wrap(const NodeRef * expr, const boost::python::list & assumptions, const SolverRef * solver){
    smt::TermVec assumpts;
    for(ssize_t i = 0; i < len(assumptions); ++i)  {
      boost::python::extract<NodeRef *>  aspt(assumptions[i]);
      if(aspt.check())
        assumpts.push_back(aspt()->node);
      else
        throw PyWASIMException(PyExc_RuntimeError, "Expecting list of NodeRef in expr_simplify_ite");
    }
    return new NodeRef(expr_simplify_ite(expr->node, assumpts, solver->solver), solver->solver);
  }

  /* TODO : tracemgr */

  /* TODO : TraverseBranchingNode */

  /* TODO : SymbolicTraverse */

} // end namespace wasim

BOOST_PYTHON_MODULE(pywasimbase)
{
  using namespace boost::python;
  using namespace wasim;

  // setup the exception translator.
  register_exception_translator
      <PyWASIMException>(translateWASIMException);

  class_<NodeType>("NodeType")
    .add_property("bitwidth", &NodeType::bitwidth)
    .add_property("arity", &NodeType::arity)
    .def("isBool", &NodeType::isBool)
    .def("isBitvector", &NodeType::isBitvector)
    .def("isArray", &NodeType::isArray)
    .def("__repr__", &NodeType::to_string)
    .def("__hash__", &NodeType::hash)
  ;

  class_<smt::Op>("SmtOp")
    .def("__repr__", &smt::Op::to_string )
  ;

  /* HZ: There is a caveat for NodeRef in a Python dictionary
  because we overload __hash__ and __eq__
  So two terms that are merged in Boolector/Bitwuzla will have the same hash
  and __eq__ will return an object that is treated as True,
  so it seems that two terms merged in Boolector/Bitwuzla will also be merged
  in the dictionary/set. (Which is probably the desired behavior)
   */

  class_<NodeRef>("NodeRef")
    .def("is_symbol", &NodeRef::is_symbol)
    .def("is_value", &NodeRef::is_value)
    .def("to_int", &NodeRef::to_int)
    .def("to_string", &NodeRef::to_string)
    .def("get_type", &NodeRef::getType, return_value_policy<manage_new_object>())
    .def("get_op", &NodeRef::get_op)
    .def("substitute", &NodeRef::substitute, return_value_policy<manage_new_object>() )
    .def("args", &NodeRef::args)
    .def("__hash__", &NodeRef::hash)
    .def("__repr__", &NodeRef::short_str)
    .def("__str__", &NodeRef::to_string)
    .def("get_solver", &NodeRef::get_solver, return_value_policy<manage_new_object>() )
    .def("get_vars", &NodeRef::get_vars)
    .def("to_smt2_fun", &NodeRef::to_smt2_fun)
    .def("__copy__", &NodeRef::python_copy, return_value_policy<manage_new_object>() )
    .def("__invert__", &NodeRef::complement, return_value_policy<manage_new_object>() )
    .def("__neg__", &NodeRef::negate, return_value_policy<manage_new_object>() )
    .def("__and__", &NodeRef::logicalAnd, return_value_policy<manage_new_object>() )
    .def("__and__", &NodeRef::logicalAndInt, return_value_policy<manage_new_object>() )
    .def("__rand__", &NodeRef::logicalAndRInt, return_value_policy<manage_new_object>() )
    .def("__or__", &NodeRef::logicalOr, return_value_policy<manage_new_object>() )
    .def("__or__", &NodeRef::logicalOrInt, return_value_policy<manage_new_object>() )
    .def("__ror__", &NodeRef::logicalOrRInt, return_value_policy<manage_new_object>() )
    
    .def("__xor__", &NodeRef::logicalXor,
          return_value_policy<manage_new_object>())
    .def("__xor__", &NodeRef::logicalXorInt,
          return_value_policy<manage_new_object>())
    .def("__rxor__", &NodeRef::logicalXorRInt,
          return_value_policy<manage_new_object>())

      // arithmetic operators.
      .def("__add__", &NodeRef::add, return_value_policy<manage_new_object>())
      .def("__add__", &NodeRef::addInt,
           return_value_policy<manage_new_object>())
      .def("__radd__", &NodeRef::raddInt,
           return_value_policy<manage_new_object>())
      .def("__sub__", &NodeRef::sub, return_value_policy<manage_new_object>())
      .def("__sub__", &NodeRef::subInt,
           return_value_policy<manage_new_object>())
      .def("__rsub__", &NodeRef::rsubInt,
           return_value_policy<manage_new_object>())
      .def("__mul__", &NodeRef::mul, return_value_policy<manage_new_object>())
      .def("__mul__", &NodeRef::mulInt,
           return_value_policy<manage_new_object>())
      .def("__rmul__", &NodeRef::rmulInt,
           return_value_policy<manage_new_object>())
      .def("__truediv__", &NodeRef::udiv, return_value_policy<manage_new_object>())
      .def("__truediv__", &NodeRef::udivInt,
           return_value_policy<manage_new_object>())
      .def("__rtruediv__", &NodeRef::rudivInt,
           return_value_policy<manage_new_object>())
      .def("__mod__", &NodeRef::urem, return_value_policy<manage_new_object>())
      .def("__mod__", &NodeRef::uremInt,
           return_value_policy<manage_new_object>())
      .def("__rmod__", &NodeRef::ruremInt,
           return_value_policy<manage_new_object>())
      .def("__lshift__", &NodeRef::shl,
           return_value_policy<manage_new_object>())
      .def("__lshift__", &NodeRef::shlInt,
           return_value_policy<manage_new_object>())
      .def("__rlshift__", &NodeRef::rshlInt,
           return_value_policy<manage_new_object>())
      .def("__rshift__", &NodeRef::lshr,
           return_value_policy<manage_new_object>())
      .def("__rshift__", &NodeRef::lshrInt,
           return_value_policy<manage_new_object>())
      .def("__rrshift__", &NodeRef::rlshrInt,
           return_value_policy<manage_new_object>())

      // comparison operators.
      .def("__eq__", &NodeRef::eq, return_value_policy<manage_new_object>())
      .def("__eq__", &NodeRef::eqInt, return_value_policy<manage_new_object>())
      .def("__ne__", &NodeRef::neq, return_value_policy<manage_new_object>())
      .def("__ne__", &NodeRef::neqInt, return_value_policy<manage_new_object>())
      .def("__lt__", &NodeRef::ult, return_value_policy<manage_new_object>())
      .def("__lt__", &NodeRef::ultInt, return_value_policy<manage_new_object>())
      .def("__le__", &NodeRef::ule, return_value_policy<manage_new_object>())
      .def("__le__", &NodeRef::uleInt, return_value_policy<manage_new_object>())
      .def("__gt__", &NodeRef::ugt, return_value_policy<manage_new_object>())
      .def("__gt__", &NodeRef::ugtInt, return_value_policy<manage_new_object>())
      .def("__ge__", &NodeRef::uge, return_value_policy<manage_new_object>())
      .def("__ge__", &NodeRef::ugeInt, return_value_policy<manage_new_object>())

      // slice operator
    //   .def("__getslice__", &NodeRef::slice,
    //        return_value_policy<manage_new_object>())
      // get bit operator
    //   .def("__getitem__", &NodeRef::getItemInt,
    //        return_value_policy<manage_new_object>())
      .def("__getitem__", &NodeRef::getitem_dispatch, return_value_policy<manage_new_object>())
  ;

  def("is_sat", &NodeRef::is_sat);
  def("same_expr", &NodeRef::same_expr);
  def("make_term", &NodeRef::make_term,
          return_value_policy<manage_new_object>());
  // memory write.
  def("store", &NodeRef::Store_operator,
          return_value_policy<manage_new_object>());
  def("read", &NodeRef::Load_operator,
          return_value_policy<manage_new_object>());
  // logical operators.
  def("nand", &NodeRef::logicalNand,
          return_value_policy<manage_new_object>());
  def("nor", &NodeRef::logicalNor,
          return_value_policy<manage_new_object>());
  def("xnor", &NodeRef::logicalXnor,
          return_value_policy<manage_new_object>());

  // arithmetic operators.
  def("sdiv", &NodeRef::sdiv,
          return_value_policy<manage_new_object>());
  def("sdiv", &NodeRef::sdivInt,
          return_value_policy<manage_new_object>());
  def("sdiv", &NodeRef::rsdivInt,
          return_value_policy<manage_new_object>()); 
  def("smod", &NodeRef::smod,
          return_value_policy<manage_new_object>()); 
  def("smod", &NodeRef::smodInt,
          return_value_policy<manage_new_object>()); 
  def("smod", &NodeRef::rsmodInt,
          return_value_policy<manage_new_object>()); 
  def("srem", &NodeRef::srem,
          return_value_policy<manage_new_object>()); 
  def("srem", &NodeRef::sremInt,
          return_value_policy<manage_new_object>()); 
  def("srem", &NodeRef::rsremInt,
          return_value_policy<manage_new_object>()); 
  def("ashr", &NodeRef::ashr,
          return_value_policy<manage_new_object>()); 
  def("ashr", &NodeRef::ashrInt,
          return_value_policy<manage_new_object>()); 
  def("ashr", &NodeRef::rashrInt,
          return_value_policy<manage_new_object>()); 
  // manipulate operators.
  def("concat", &NodeRef::concat,
          return_value_policy<manage_new_object>());
  def("concat", &NodeRef::concatList,
          return_value_policy<manage_new_object>());
  def("lrotate", &NodeRef::lrotate,
          return_value_policy<manage_new_object>());
  def("rrotate", &NodeRef::rrotate,
          return_value_policy<manage_new_object>());
  def("zero_extend", &NodeRef::zero_extend,
          return_value_policy<manage_new_object>());
  def("sign_extend", &NodeRef::sign_extend,
          return_value_policy<manage_new_object>());
  def("extract", &NodeRef::Extract_operator,
          return_value_policy<manage_new_object>());

  // comparison operators.
  def("slt", &NodeRef::slt,
          return_value_policy<manage_new_object>());
  def("slt", &NodeRef::sltInt,
          return_value_policy<manage_new_object>());
  def("slt", &NodeRef::sltIntR,
          return_value_policy<manage_new_object>());
  def("sle", &NodeRef::sle,
          return_value_policy<manage_new_object>());
  def("sle", &NodeRef::sleInt,
          return_value_policy<manage_new_object>());
  def("sle", &NodeRef::sleIntR,
          return_value_policy<manage_new_object>());
  def("sgt", &NodeRef::sgt,
          return_value_policy<manage_new_object>());
  def("sgt", &NodeRef::sgtInt,
          return_value_policy<manage_new_object>());
  def("sgt", &NodeRef::sgtIntR,
          return_value_policy<manage_new_object>());
  def("sge", &NodeRef::sge,
          return_value_policy<manage_new_object>());
  def("sge", &NodeRef::sgeInt,
          return_value_policy<manage_new_object>());
  def("sge", &NodeRef::sgeIntR,
          return_value_policy<manage_new_object>());
  // ite.
  def("ite", &NodeRef::ITE_operator,
          return_value_policy<manage_new_object>());


  class_<StateRef>("StateRef")
    .def("get_assumptions",&StateRef::get_assumptions)
    .def("set_assumptions",&StateRef::set_assumptions)
    .def("clone", &StateRef::clone, return_value_policy<manage_new_object>())
    .def("__copy__", &StateRef::clone, return_value_policy<manage_new_object>())
    .def("get_assumption_interps", &StateRef::get_assumption_interps)
    .def("set_assumption_interps", &StateRef::set_assumption_interps)
    .def("get_vars", &StateRef::get_vars)
    .def("get_varnames", &StateRef::get_varnames)
    .def("get_sv", &StateRef::get_sv)
    .def("set_sv", &StateRef::set_sv)
  ;

  class_<SolverRef>("SolverRef")
    .def("push", &SolverRef::push)
    .def("pop", &SolverRef::pop)
    .def("assert_formula", &SolverRef::assert_formula)
    .def("check_sat", &SolverRef::check_sat)
    .def("check_sat_assuming", &SolverRef::check_sat_assuming)
    .def("get_value", &SolverRef::get_value, return_value_policy<manage_new_object>())
    .def("make_bool", &SolverRef::make_bool, return_value_policy<manage_new_object>())
    .def("make_bvsort", &SolverRef::make_bvsort, return_value_policy<manage_new_object>())
    .def("make_constant", &SolverRef::make_constant, return_value_policy<manage_new_object>())
  ;

  class_<TransSys>("TransSys", init<const std::string &>())
    .def(init<const std::string &, const std::string &>())
    .def(init<const std::string &, SolverRef *>())
    .def(init<const std::string &, SolverRef *, const std::string &>())
    .def("curr", &TransSys::curr, return_value_policy<manage_new_object>())
    .def("next", &TransSys::next, return_value_policy<manage_new_object>())
    .def("is_curr_var", &TransSys::is_curr_var)
    .def("is_next_var", &TransSys::is_next_var)
    .def("is_input_var", &TransSys::is_input_var)
    .def("get_name", &TransSys::get_name)
    .def("lookup", &TransSys::lookup, return_value_policy<manage_new_object>())
    .def("get_solver", &TransSys::get_solver, return_value_policy<manage_new_object>())
    .def("statevars", &TransSys::statevars)
    .def("inputvars", &TransSys::inputvars)
    .def("output_terms", &TransSys::output_terms)
    .def("state_updates", &TransSys::state_updates)
    .def("named_terms", &TransSys::named_terms)
    .def("constraints", &TransSys::constraints)
    .def("prop", &TransSys::prop)
    .def("init_constants", &TransSys::init_constants)
    .def("init", &TransSys::init, return_value_policy<manage_new_object>())
    .def("trans", &TransSys::trans, return_value_policy<manage_new_object>())
    .def("is_functional", &TransSys::is_functional)
    .def("is_deterministic", &TransSys::is_deterministic)
    .def("only_curr", &TransSys::only_curr)
    .def("no_next", &TransSys::no_next)
    .def("__copy__", &TransSys::clone, return_value_policy<manage_new_object>())
  ;

  class_<Symsimulator>("Symsimulator", init<TransSys *>())
    .def("tracelen", &Symsimulator::tracelen)
    .def("all_assumptions", &Symsimulator::all_assumptions)
    .def("all_assumption_interp", &Symsimulator::all_assumption_interp)

    .def("var", &Symsimulator::var, return_value_policy<manage_new_object>())
    .def("cur", &Symsimulator::cur, return_value_policy<manage_new_object>())
    .def("print_current_step", &Symsimulator::print_current_step)
    .def("print_current_step_assumptions", &Symsimulator::print_current_step_assumptions)
    .def("create_input_Xvars", &Symsimulator::create_input_Xvars)
    .def("convert", &Symsimulator::convert)
    .def("backtrack", &Symsimulator::backtrack)
    .def("init", &Symsimulator::init)
    .def("free_init", &Symsimulator::free_init)
    .def("set_current_state", &Symsimulator::set_current_state)
    .def("set_input", &Symsimulator::set_input)
    .def("undo_set_input", &Symsimulator::undo_set_input)

    .def("interpret_state_expr_on_curr_frame", &Symsimulator::interpret_state_expr_on_curr_frame, return_value_policy<manage_new_object>() )
    .def("interpret_input_and_state_expr_on_curr_frame", &Symsimulator::interpret_input_and_state_expr_on_curr_frame, return_value_policy<manage_new_object>() )
    
    .def("sim_one_step", &Symsimulator::sim_one_step)
    .def("get_Xs", &Symsimulator::get_Xs)
    .def("get_curr_state", &Symsimulator::get_curr_state, return_value_policy<manage_new_object>())
    .def("set_var", &Symsimulator::set_var, return_value_policy<manage_new_object>())
    .def("get_var", &Symsimulator::get_var, return_value_policy<manage_new_object>())
    .def("get_solver", &Symsimulator::get_solver, return_value_policy<manage_new_object>())
    .def("dump_waveform", &Symsimulator::dump_waveform)
  ;

  class_<Symsimbranch>("Symsimbranch", init<TransSys *>())
    .def("fork_branch", &Symsimbranch::fork_branch)
    .def("finish_branch", &Symsimbranch::finish_branch)
    .def("tracelen", &Symsimbranch::tracelen)
    .def("all_assumptions", &Symsimbranch::all_assumptions)
    .def("all_assumption_interp", &Symsimbranch::all_assumption_interp)
    .def("add_assumption_interpreted", &Symsimbranch::add_assumption_interpreted)
    .def("var", &Symsimbranch::var, return_value_policy<manage_new_object>())
    .def("cur", &Symsimbranch::cur, return_value_policy<manage_new_object>())
    .def("print_current_step", &Symsimbranch::print_current_step)
    .def("print_current_step_assumptions", &Symsimbranch::print_current_step_assumptions)
    .def("print_current_step_all_branches", &Symsimbranch::print_current_step_all_branches)
    .def("print_current_step_assumptions_all_branches", &Symsimbranch::print_current_step_assumptions_all_branches)
    .def("create_input_Xvars", &Symsimbranch::create_input_Xvars)
    .def("is_Xvar", &Symsimbranch::is_Xvar)
    .def("convert", &Symsimbranch::convert)
    .def("backtrack", &Symsimbranch::backtrack)
    .def("init", &Symsimbranch::init)
    .def("free_init", &Symsimbranch::free_init)
    .def("set_current_state", &Symsimbranch::set_current_state)
    .def("set_input", &Symsimbranch::set_input)
    .def("undo_set_input", &Symsimbranch::undo_set_input)

    .def("interpret_state_expr_on_curr_frame", &Symsimbranch::interpret_state_expr_on_curr_frame, return_value_policy<manage_new_object>() )
    .def("interpret_input_and_state_expr_on_curr_frame", &Symsimbranch::interpret_input_and_state_expr_on_curr_frame, return_value_policy<manage_new_object>() )
    
    .def("sim_one_step", &Symsimbranch::sim_one_step)
    .def("get_Xs", &Symsimbranch::get_Xs)
    .def("get_curr_state", &Symsimbranch::get_curr_state, return_value_policy<manage_new_object>())
    .def("set_var", &Symsimbranch::set_var, return_value_policy<manage_new_object>())
    .def("get_var", &Symsimbranch::get_var, return_value_policy<manage_new_object>())
    .def("get_solver", &Symsimbranch::get_solver, return_value_policy<manage_new_object>())
    .def("dump_waveform", &Symsimbranch::dump_waveform)
  ;

  // public func
  def("expr_simplify_ite", &wasim::expr_simplify_ite_wrap, return_value_policy<manage_new_object>());

}
