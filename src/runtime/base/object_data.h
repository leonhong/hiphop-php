/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_OBJECT_DATA_H__
#define __HPHP_OBJECT_DATA_H__

#include <runtime/base/util/countable.h>
#include <runtime/base/types.h>
#include <runtime/base/memory/unsafe_pointer.h>
#include <runtime/base/macros.h>
#include <runtime/base/runtime_error.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Needed for eval
namespace Eval {
class MethodStatement;
class FunctionCallExpression;
class VariableEnvironment;
}


/**
 * Base class of all user-defined classes. All data members and methods in
 * this class should start with "o_" or "os_" to avoid name conflicts.
 *
 * Calling sequence:
 *
 * 1. Statically resolved properties and methods will be statically called.
 * 2. Dynamic properties:
 *    o_get() -> t___get() as fallback
 *    o_lval() -> o_set() -> t___set() as fallback
 * 3. Dynamic methods:
 *    o_invoke() -> t___call() as fallback
 * 4. Auto-generated jump-tables:
 *    o_exists()
 *    o_get()
 *    o_set()
 *    o_lval()
 *    o_invoke()
 */
class ObjectData : public Countable {
 public:
  enum Attribute {
    InConstructor = 1, // __construct()
    InDestructor  = 2, // __destruct()
    HasSleep      = 4, // __sleep()
    InSet         = 8, // __set()
    InGet         = 16, // __get()
  };

  ObjectData(bool isResource = false);
  virtual ~ObjectData(); // all PHP classes need virtual tables

  bool getAttribute(Attribute attr) const { return o_attribute & attr; }
  void setAttribute(Attribute attr) const { o_attribute |= attr;}
  void clearAttribute(Attribute attr) const { o_attribute &= ~attr;}
  bool inDtor() { return getAttribute(InDestructor); }
  bool inCtor() { return getAttribute(InConstructor); }
  bool inCtorDtor() { return inCtor() || inDtor(); }
  void setInDtor() { setAttribute(InDestructor); }
  bool gasInCtor(bool inCtor) { // get and set InConstructor
    bool oldInCtor = getAttribute(InConstructor);
    if (inCtor) {
      setAttribute(InConstructor);
    } else {
      clearAttribute(InConstructor);
    }
    return oldInCtor;
  }

  /**
   * o_instanceof() can be used for both classes and interfaces. Note
   * that o_instanceof() has not been thoroughly tested with redeclared
   * classes or classes that have a redeclared ancestor in the inheritance
   * hierarchy. It is also worth noting that o_instanceof will always return
   * false for classes that are descendents of ResourceData.
   */
  virtual bool o_instanceof(const char *s) const = 0;
  virtual ObjectData *getRedeclaredParent() const { return 0; }

  // class info
  virtual CStrRef o_getClassName() const = 0;
  virtual bool isResource() const { return false;}
  virtual int64 o_toInt64() const { return 1;}
  bool o_isClass(const char *s) const;
  int o_getId() const { return o_id;}

  template<typename T>
  T *bindClass(ThreadInfo *info) {
    bindThis(info);
    return dynamic_cast<T*>(this);
  }
  void bindThis(ThreadInfo *info);

  virtual void init() {}
  ObjectData *create() { CountableHelper h(this); init(); return this;}
  ObjectData *dynCreate(const Array &params, bool init = true) {
    create();
    if (init) {
      CountableHelper h(this);
      dynConstruct(params);
    }
    return this;
  }
  virtual void dynConstruct(CArrRef params) {}
  virtual void dynConstructFromEval(Eval::VariableEnvironment &env,
                                    const Eval::FunctionCallExpression *call);
  virtual void release() { destruct(); delete this; } // for SmartPtr<T>
  virtual void destruct() {}

  virtual const
    Eval::MethodStatement *getMethodStatement(const char* name) const;

  static Variant os_getInit(CStrRef s);
  // static methods and properties
  static Variant os_get(CStrRef s);
  static Variant &os_lval(CStrRef s);
  static Variant os_invoke(const char *c, MethodIndex, const char *s,
                           CArrRef params, int64 hash, bool fatal = true);
  static Variant os_invoke_mil(const char *c, const char *s,
                               CArrRef params, int64 hash, bool fatal = true);
  static Variant os_constant(const char *s);

  static Variant os_invoke_from_eval(const char *c, const char *s,
                                     Eval::VariableEnvironment &env,
                                     const Eval::FunctionCallExpression *call,
                                     int64 hash,
                                     bool fatal /* = true */);


  // properties
  virtual Array o_toArray() const;
  virtual Array o_toIterArray(CStrRef context, bool getRef = false);
  virtual Array o_getDynamicProperties() const;
  virtual bool o_exists(CStrRef s, CStrRef context = null_string) const;
  virtual bool o_existsPublic(CStrRef s) const;
  virtual Variant o_get(CStrRef s, bool error = true,
                        CStrRef context = null_string);
  virtual Variant o_getPublic(CStrRef s, bool error = true);
  virtual Variant o_getUnchecked(CStrRef s, CStrRef context = null_string);
  virtual Variant o_set(CStrRef s, CVarRef v, bool forInit = false,
                        CStrRef context = null_string);
  virtual Variant o_setPublic(CStrRef s, CVarRef v, bool forInit);
  virtual Variant &o_lval(CStrRef s, CStrRef context = null_string);
  virtual Variant &o_lvalPublic(CStrRef s);

  virtual void o_setArray(CArrRef properties);
  virtual void o_getArray(Array &props) const {}

  /**
   * This is different from o_exists(), which is isset() semantics. This one
   * is property_exists() semantics that check whether it was unset before.
   * This is used for deciding what property_exists() returns and whether or
   * not this property should be part of an iteration in foreach ($obj as ...)
   */
  bool o_propExists(CStrRef s, CStrRef context = null_string);

  static Object FromArray(ArrayData *properties);

  CVarRef set(CStrRef s, CVarRef v);

  // methods
  virtual Variant o_invoke(MethodIndex, const char *s,
                           CArrRef params, int64 hash, bool fatal = true);
  virtual Variant o_invoke_mil(const char *s,
                               CArrRef params, int64 hash, bool fatal = true);
  virtual Variant o_root_invoke(MethodIndex, const char *s,
                                CArrRef params, int64 hash, bool fatal = false);
  virtual Variant o_root_invoke_mil(const char *s,
                                    CArrRef params, int64 hash,
                                    bool fatal = false);
  virtual Variant o_invoke_ex(const char *clsname, MethodIndex,
                              const char *s,
                              CArrRef params, int64 hash, bool fatal = true);
  virtual Variant o_invoke_ex_mil(const char *clsname, const char *s,
                                  CArrRef params, int64 hash,
                                  bool fatal = true);

  virtual Variant o_invoke_few_args(MethodIndex, const char *s,
                                    int64 hash, int count,
                                    INVOKE_FEW_ARGS_DECL_ARGS);
  virtual Variant o_invoke_few_args_mil(const char *s,
                                        int64 hash, int count,
                                        INVOKE_FEW_ARGS_DECL_ARGS);

  virtual Variant o_root_invoke_few_args(MethodIndex, const char *s,
                                         int64 hash, int count,
                                         INVOKE_FEW_ARGS_DECL_ARGS);
  virtual Variant o_root_invoke_few_args_mil(const char *s,
                                             int64 hash, int count,
                                             INVOKE_FEW_ARGS_DECL_ARGS);
  virtual Variant o_invoke_from_eval(const char *s,
                                     Eval::VariableEnvironment &env,
                                     const Eval::FunctionCallExpression *call,
                                     int64 hash,
                                     bool fatal /* = true */);

  // method invocation with CStrRef
  Variant o_invoke(CStrRef s, CArrRef params, int64 hash = -1,
                   bool fatal = true);
  Variant o_root_invoke(CStrRef s, CArrRef params, int64 hash = -1,
                        bool fatal = false);
  Variant o_invoke_few_args(CStrRef s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  Variant o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                 INVOKE_FEW_ARGS_DECL_ARGS);

  // misc
  Variant o_throw_fatal(const char *msg);
  virtual void serialize(VariableSerializer *serializer) const;
  virtual void dump() const;
  virtual ObjectData *clone();
  virtual void setRoot(ObjectData *root) {}
  virtual ObjectData *getRoot();
  /**
   * If __call is defined, then this gets overridden to call it.
   * Otherwise, it just throws if fatal else returns false.
   */
  virtual Variant doCall(Variant v_name, Variant v_arguments, bool fatal);
  virtual Variant doRootCall(Variant v_name, Variant v_arguments, bool fatal);

  virtual Variant doGet(Variant v_name, bool error);
  virtual bool doIsSet(CStrRef prop, CStrRef context = null_string);
  virtual bool doEmpty(CStrRef prop, CStrRef context = null_string);
  // magic methods
  // __construct is handled in a special way
  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual Variant &___lval(Variant v_name);
  virtual Variant &___offsetget_lval(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual Variant t___set_state(Variant v_properties);
  virtual String t___tostring();
  virtual Variant t___clone();

  /**
   * Marshaling/Unmarshaling between request thread and fiber thread.
   */
  virtual Object fiberMarshal(FiberReferenceMap &refMap) const;
  virtual Object fiberUnmarshal(FiberReferenceMap &refMap) const;

 protected:
  virtual ObjectData* cloneImpl() = 0;
  void cloneSet(ObjectData *clone);
  virtual bool php_sleep(Variant &ret);
  static Array collectArgs(int count, INVOKE_FEW_ARGS_IMPL_ARGS) ;

 private:
  ObjectData(const ObjectData &) { ASSERT(false);}

 protected:
  int o_id;                      // a numeric identifier of this object
  mutable Array *o_properties;   // dynamic properties
 private:
  mutable int16  o_attribute;    // vairous flags

#ifdef FAST_REFCOUNT_FOR_VARIANT
 private:
  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(ObjectData, _count) == FAST_REFCOUNT_OFFSET);
  }
#endif
};


typedef ObjectData c_ObjectData; // purely for easier code generation

class ExtObjectData : public ObjectData {
public:
  ExtObjectData() : root(this) {}
  Variant o_root_invoke(MethodIndex, const char *s, CArrRef ps, int64 h,
                        bool f = true);
  Variant o_root_invoke_few_args(MethodIndex, const char *s, int64 h, int count,
                          INVOKE_FEW_ARGS_DECL_ARGS);
  virtual void setRoot(ObjectData *r) { root = r; }
  virtual ObjectData *getRoot() { return root; }
protected: ObjectData *root;

};

// Callback structure for functions related to static methods
struct ObjectStaticCallbacks {
  Variant (*os_getInit)(CStrRef s);
  Variant (*os_get)(CStrRef s);
  Variant &(*os_lval)(CStrRef s);
  Variant (*os_invoke)(const char *c, MethodIndex, const char *s,
                           CArrRef params, int64 hash, bool fatal);
  Variant (*os_constant)(const char *s);
};

///////////////////////////////////////////////////////////////////////////////
// Calculate item sizes for object allocators

#define WORD_SIZE sizeof(void *)
#define ALIGN_WORD(n) ((n) + (WORD_SIZE - (n) % WORD_SIZE) % WORD_SIZE)
#define UNIT_SIZE sizeof(ObjectData)

template<int M>
class ItemSize {
 public:
  enum {
    prev = (M + M + 3) / 3 >= UNIT_SIZE ? (M + M + 3) / 3 : UNIT_SIZE,
    value = (ItemSize<prev>::value < M ?
             ALIGN_WORD(ItemSize<prev>::value + (ItemSize<prev>::value >> 1)) :
             ItemSize<prev>::value)
  };
};

template<>
class ItemSize<UNIT_SIZE> {
 public:
  enum {
    prev = 0,
    value = UNIT_SIZE
  };
};

///////////////////////////////////////////////////////////////////////////////
// Attribute helper
class AttributeSetter {
public:
  AttributeSetter(ObjectData::Attribute a, ObjectData *o) : m_a(a), m_o(o) {
    o->setAttribute(a);
  }
  ~AttributeSetter() {
    m_o->clearAttribute(m_a);
  }
private:
  ObjectData::Attribute m_a;
  ObjectData *m_o;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OBJECT_DATA_H__
