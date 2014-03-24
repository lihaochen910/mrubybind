// Do not modify this file directly, this is generated
/**
 * mrubybind - Binding library for mruby/C++
 *
 * Usage:
 *   1. Prepare a function which you want to call from mruby:
 *     > int square(int x)  { return x * x; }
 *
 *   2. Create MrubyBind instance:
 *     > MrubyBind b(mirb)
 *
 *   3. Bind a function:
 *     > b.bind("square", square);
 *
 *   4. You can call it from mruby:
 *     > puts square(1111)  #=> 1234321
 *
 *   There are other methods to bind constant/class/instance method in
 *   MrubyBind. Please see the definition of MrubyBind
 *   (the bottom of this file), or README.
 */
#ifndef __MRUBYBIND_H__
#define __MRUBYBIND_H__

#ifndef __cplusplus
#error mrubybind can be used from C++ only.
#endif

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"
//#include "mrubybind_types.h"
#include "mruby/string.h"
#include "mruby/proc.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/variable.h"
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <assert.h>
#include <iostream>
#define ASSERT(expr)  assert(expr)

namespace mrubybind {

extern const char* untouchable_table;
extern const char* untouchable_object;

class MrubyArenaStore{
    mrb_state* mrb;
    int ai;
public:
    MrubyArenaStore(mrb_state* mrb)
    {
        this->mrb = mrb;
        this->ai = mrb_gc_arena_save(mrb);
    }
    
    ~MrubyArenaStore()
    {
        mrb_gc_arena_restore(mrb, ai);
    }

};

class MrubyBindStatus{

public:

    struct Data;
    typedef std::shared_ptr<Data> Data_ptr;
    typedef std::map<mrb_state*, Data_ptr > Table;
    
    static Table& get_living_table(){
        static Table table;
        return table;
    }

    struct Data{
        mrb_state* mrb;
        mrb_value avoid_gc_table;
        
        Data(){
            
        }
        ~Data(){
            
        }
        
        mrb_state* get_mrb(){
            return mrb;
        }
        
        mrb_value get_avoid_gc_table(){
            return avoid_gc_table;
        }
        
    };
    
    MrubyBindStatus(){
        
    }

    MrubyBindStatus(mrb_state* mrb, mrb_value avoid_gc_table){
    
        Table& living_table = get_living_table();
        data = std::make_shared<Data>();
        data->mrb = mrb;
        data->avoid_gc_table = avoid_gc_table;
        living_table[mrb] = data;
    }
    
    ~MrubyBindStatus(){
    
        Table& living_table = MrubyBindStatus::get_living_table();
        living_table.erase(data->mrb);
        data->mrb = NULL;
        
    }
    
    static bool is_living(mrb_state* mrb){
        Table& living_table = get_living_table();
        if(living_table.find(mrb) != living_table.end()){
            return living_table[mrb].get();
        }
        return false;
    }
    
    static Data_ptr search(mrb_state* mrb){
        Table& living_table = get_living_table();
        if(living_table.find(mrb) != living_table.end()){
            return living_table[mrb];
        }
        return Data_ptr(NULL);
    }
    
    
    
private:
    std::shared_ptr<Data> data;
};

template<class T> class Deleter{
    MrubyBindStatus::Data_ptr mrbsp;
    mrb_value v_;
public:
    Deleter()
    {
        
    }

    Deleter(mrb_state* mrb, mrb_value v){
        mrbsp = MrubyBindStatus::search(mrb);
        mrb_value avoid_gc_table = mrbsp->get_avoid_gc_table();
        mrb_value s = mrb_hash_get(mrb, avoid_gc_table, v);
        if(mrb_test(v) && mrb_obj_equal(mrb, v, s)){
            mrb_value a = mrb_ary_new(mrb);
            mrb_ary_push(mrb, a, v);
            v = a;
        }
        mrb_hash_set(mrb, avoid_gc_table, v, v);
        v_ = v;
    }
    ~Deleter(){

    }
    void operator()(T* p) const {
        if(mrbsp.get()){
            mrb_state* mrb = mrbsp->get_mrb();
            if(mrb){
                mrb_value avoid_gc_table = mrbsp->get_avoid_gc_table();
                mrb_hash_delete_key(mrb, avoid_gc_table, v_);
            }
        }
        if(p){
            delete p;
        }
    }

};

template<class T> using obj_ptr = std::shared_ptr<T>;
//template<class T> using FuncPtr = std::shared_ptr<std::function<T> >;

template<class T> class FuncPtr{
    std::shared_ptr<std::function<T> > p;
public:
    FuncPtr(){

    }
    template<class D>FuncPtr(std::function<T>* pt, D d) : p(pt, d){

    }
    ~FuncPtr(){

    }
    std::shared_ptr<std::function<T> >& ref(){
        return p;
    }
    std::function<T>& func(){
        if(!p.get()){
            throw std::runtime_error("empty function.");
        }
        return *p.get();
    }
    operator bool() {
        if(!p.get()){
            return false;
        }
        return (bool)*p.get();
    }
    void reset(){
        p.reset();
    }
    template<class Y> void reset(Y* y){
        p.reset(y);
    }
    template<class Y, class D> void reset(Y* y, D d){
        p.reset(y, d);
    }
    template<class Y, class D, class A> void reset(Y* y, D d, A a){
        p.reset(y, d, a);
    }
};

template<class T> Deleter<T> set_avoid_gc(mrb_state* mrb, mrb_value v){
    return Deleter<T>(mrb, v);
}

template<class T> obj_ptr<T> make_obj_ptr(Deleter<T> d, T t){
    T* pt = new T();
    *pt = t;
    return obj_ptr<T>(pt, d);
}

template<class T> FuncPtr<T> make_FuncPtr(Deleter<std::function<T> > d, std::function<T> t){
    std::function<T>* pt = new std::function<T>();
    *pt = t;
    return FuncPtr<T>(pt, d);
}

template <class T>
struct Type;

class MrubyRef{
    mrb_state* mrb;
    std::shared_ptr<mrb_value> v;
public:
    
    MrubyRef();
    MrubyRef(mrb_state* mrb, const mrb_value& v);
    ~MrubyRef();
    
    mrb_value get_v()const;
    bool empty() const;
    bool test() const;
    bool obj_equal(const MrubyRef& r) const;
    std::string to_s() const;
    int to_i() const;
    float to_float() const;
    double to_double() const;
    
    MrubyRef call(std::string name){
        MrubyArenaStore mas(mrb);
        return MrubyRef(mrb, mrb_funcall(mrb, *(this->v.get()), name.c_str(), 0));
    }
    
#include "mrubybind_call_generated.h"

};

//===========================================================================
// C <-> mruby type converter.

//struct Type {
  //static int check(mrb_value v) = 0;
  //static int get(mrb_value v) = 0;
  //static mrb_value ret(mrb_state*, int i) = 0;
//};

// Fixnum
template<>
struct Type<int> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_fixnum_p(v) || mrb_float_p(v); }
  static int get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_fixnum_p(v) ? mrb_fixnum(v) : mrb_float(v); }
  static mrb_value ret(mrb_state*, int i) { return mrb_fixnum_value(i); }
};

template<>
struct Type<unsigned int> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_fixnum_p(v) || mrb_float_p(v); }
  static unsigned int get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_fixnum_p(v) ? mrb_fixnum(v) : mrb_float(v); }
  static mrb_value ret(mrb_state*, unsigned int i) { return mrb_fixnum_value(i); }
};

// float
template<>
struct Type<float> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_float_p(v) || mrb_fixnum_p(v); }
  static float get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_float_p(v) ? mrb_float(v) : mrb_fixnum(v); }
  static mrb_value ret(mrb_state* mrb, float f) { return mrb_float_value(mrb, f); }
};

// double
template<>
struct Type<double> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_float_p(v) || mrb_fixnum_p(v); }
  static double get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_float_p(v) ? mrb_float(v) : mrb_fixnum(v); }
  static mrb_value ret(mrb_state* mrb, double f) { return mrb_float_value(mrb, f); }
};

// String
template<>
struct Type<const char*> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_string_p(v); }
  static const char* get(mrb_state* mrb, mrb_value v) { (void)mrb; return RSTRING_PTR(v); }
  static mrb_value ret(mrb_state* mrb, const char* s) { return mrb_str_new_cstr(mrb, s); }
};

template<>
struct Type<std::string> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_string_p(v); }
  static const std::string get(mrb_state* mrb, mrb_value v) { (void)mrb; return std::string(RSTRING_PTR(v), RSTRING_LEN(v)); }
  static mrb_value ret(mrb_state* mrb, const std::string& s) { return mrb_str_new(mrb, s.c_str(), s.size()); }
};

template<>
struct Type<const std::string> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_string_p(v); }
  static const std::string get(mrb_state* mrb, mrb_value v) { (void)mrb; return std::string(RSTRING_PTR(v), RSTRING_LEN(v)); }
  static mrb_value ret(mrb_state* mrb, const std::string& s) { return mrb_str_new(mrb, s.c_str(), s.size()); }
};

template<>
struct Type<const std::string&> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_string_p(v); }
  static const std::string get(mrb_state* mrb, mrb_value v) { (void)mrb; return std::string(RSTRING_PTR(v), RSTRING_LEN(v)); }
  static mrb_value ret(mrb_state* mrb, const std::string& s) { return mrb_str_new(mrb, s.c_str(), s.size()); }
};

// Boolean
template<>
struct Type<bool> {
  static const char TYPE_NAME[];
  static int check(mrb_value /*v*/) { return 1; }
  static bool get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_test(v); }
  static mrb_value ret(mrb_state* /*mrb*/, bool b) { return b ? mrb_true_value() : mrb_false_value(); }
};

// Raw pointer
template<>
struct Type<void*> {
  static const char TYPE_NAME[];
  static int check(mrb_value v) { return mrb_voidp_p(v); }
  static void* get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_voidp(v); }
  static mrb_value ret(mrb_state* mrb, void* p) { return mrb_voidp_value(mrb, p); }
};

// Function
struct TypeFuncBase{
    static const char TYPE_NAME[];
};

template<class R>
struct Type<FuncPtr<R()> > :public TypeFuncBase {
  static int check(mrb_value v) { return mrb_type(v) == MRB_TT_PROC; }
  static FuncPtr<R()> get(mrb_state* mrb, mrb_value v) {
      Deleter<std::function<R()> > d = set_avoid_gc<std::function<R()> >(mrb, v);
      return make_FuncPtr<R()>(d, [=](){
          MrubyArenaStore mas(mrb);
          return Type<R>::get(mrb, mrb_yield(mrb, v, mrb_nil_value()));
      });
  }
  static mrb_value ret(mrb_state* mrb, FuncPtr<R()> p) {
      // don't call.
      throw std::runtime_error("don't call Type<FuncPtr<R()> >::ret");
      (void)mrb; (void)p; return mrb_nil_value();
  }
};

template<>
struct Type<FuncPtr<void()> > :public TypeFuncBase {
  static int check(mrb_value v) { return mrb_type(v) == MRB_TT_PROC; }
  static FuncPtr<void()> get(mrb_state* mrb, mrb_value v) {
      Deleter<std::function<void()> > d = set_avoid_gc<std::function<void()> >(mrb, v);
      return make_FuncPtr<void()>(d, [=](){
          MrubyArenaStore mas(mrb);
          mrb_yield(mrb, v, mrb_nil_value());
      });
  }
  static mrb_value ret(mrb_state* mrb, FuncPtr<void()> p) {
      // don't call.
      throw std::runtime_error("don't call Type<FuncPtr<void()> >::ret");
      (void)mrb; (void)p; return mrb_nil_value();
  }
};

// mruby value
template<>
struct Type<MrubyRef> {
  static const char TYPE_NAME[];
  static int check(mrb_value) { return 1; }
  static MrubyRef get(mrb_state* mrb, mrb_value v) { (void)mrb; return MrubyRef(mrb, v); }
  static mrb_value ret(mrb_state*, MrubyRef r) { return r.get_v(); }
};


#include "mrubybind_types_generated.h"

//===========================================================================
// Binder

// Template class for Binder.
// Binder template class is specialized with type.
template <class T>
struct Binder {
  // Template specialization.
  //static mrb_value call(mrb_state* mrb, void* p, mrb_value* args, int narg) = 0;
};

// Template class for Binder.
// Binder template class is specialized with type.
template <class C>
struct ClassBinder {
  static struct mrb_data_type type_info;
  static void dtor(mrb_state*, void* p) {
    C* instance = static_cast<C*>(p);
    delete instance;
  }

  // Template specialization.
  //static void ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
};
template<class C>
mrb_data_type ClassBinder<C>::type_info = { "???", dtor };

// Other Class
struct TypeClassBase{
    static const char TYPE_NAME[];
};

template<class T> struct Type :public TypeClassBase {
    static std::string class_name;
    static int check(mrb_value v) { 
        return mrb_type(v) == MRB_TT_DATA; 
    }
    static T get(mrb_state* mrb, mrb_value v) { 
            (void)mrb; return *(T*)DATA_PTR(v); 
        }
    static mrb_value ret(mrb_state* mrb, T t) { 
        RClass* cls;
        mrb_value v;
        cls = mrb_class_get(mrb, class_name.c_str());
        v = mrb_class_new_instance(mrb, 0, NULL, cls);
        DATA_TYPE(v) = &ClassBinder<T>::type_info;
        T* nt = new T();
        *nt = t;
        DATA_PTR(v) = nt;
        return v;
    }
};

template<class T> std::string Type<T>::class_name = "";

//
mrb_value raise(mrb_state *mrb, int parameter_index,
                const char* required_type_name, mrb_value value);

// Includes generated template specialization.
//#include "mrubybind.inc"
// This file is generated from gen_template.rb
#define ARG(mrb, i)  Type<P##i>::get(mrb, args[i])
#define CHECK(i)  {if(!Type<P##i>::check(args[i])) return RAISE(i);}
#define RAISE(i)  raise(mrb, i, Type<P##i>::TYPE_NAME, args[i])

// void f(void);
template<>
struct Binder<void (*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(mrb);(void)(args);(void)(narg);
    void (*fp)(void) = (void (*)(void))func_ptr;
    fp();
    return mrb_nil_value();
  }
};

// R f(void);
template<class R>
struct Binder<R (*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(mrb);(void)(args);(void)(narg);
    R (*fp)(void) = (R (*)(void))func_ptr;
    R result = fp();
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(void);
template<class C>
struct ClassBinder<C* (*)(void)> {
  static const int NPARAM = 0;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(mrb);(void)(args);(void)(narg);
    if(new_func_ptr){
        C* (*ctor)(void) = (C* (*)(void))new_func_ptr;
        C* instance = ctor();
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(void) };
template<class C>
struct ClassBinder<void (C::*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(mrb);(void)(args);(void)(narg);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(void);
    M mp = *(M*)method_pptr;
    (instance->*mp)();
    return mrb_nil_value();
  }
};

// class C { R f(void) };
template<class C, class R>
struct ClassBinder<R (C::*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(mrb);(void)(args);(void)(narg);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(void);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)();
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0);
template<class P0>
struct Binder<void (*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0);
    void (*fp)(P0) = (void (*)(P0))func_ptr;
    fp(ARG(mrb, 0));
    return mrb_nil_value();
  }
};

// R f(P0);
template<class R, class P0>
struct Binder<R (*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0);
    R (*fp)(P0) = (R (*)(P0))func_ptr;
    R result = fp(ARG(mrb, 0));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0);
template<class C, class P0>
struct ClassBinder<C* (*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0);
    if(new_func_ptr){
        C* (*ctor)(P0) = (C* (*)(P0))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0) };
template<class C, class P0>
struct ClassBinder<void (C::*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0));
    return mrb_nil_value();
  }
};

// class C { R f(P0) };
template<class C, class R, class P0>
struct ClassBinder<R (C::*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1);
template<class P0, class P1>
struct Binder<void (*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1);
    void (*fp)(P0, P1) = (void (*)(P0, P1))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1));
    return mrb_nil_value();
  }
};

// R f(P0, P1);
template<class R, class P0, class P1>
struct Binder<R (*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1);
    R (*fp)(P0, P1) = (R (*)(P0, P1))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1);
template<class C, class P0, class P1>
struct ClassBinder<C* (*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1);
    if(new_func_ptr){
        C* (*ctor)(P0, P1) = (C* (*)(P0, P1))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1) };
template<class C, class P0, class P1>
struct ClassBinder<void (C::*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1) };
template<class C, class R, class P0, class P1>
struct ClassBinder<R (C::*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2);
template<class P0, class P1, class P2>
struct Binder<void (*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2);
    void (*fp)(P0, P1, P2) = (void (*)(P0, P1, P2))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2);
template<class R, class P0, class P1, class P2>
struct Binder<R (*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2);
    R (*fp)(P0, P1, P2) = (R (*)(P0, P1, P2))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2);
template<class C, class P0, class P1, class P2>
struct ClassBinder<C* (*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2) = (C* (*)(P0, P1, P2))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2) };
template<class C, class P0, class P1, class P2>
struct ClassBinder<void (C::*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2) };
template<class C, class R, class P0, class P1, class P2>
struct ClassBinder<R (C::*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3);
template<class P0, class P1, class P2, class P3>
struct Binder<void (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3);
    void (*fp)(P0, P1, P2, P3) = (void (*)(P0, P1, P2, P3))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3);
template<class R, class P0, class P1, class P2, class P3>
struct Binder<R (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3);
    R (*fp)(P0, P1, P2, P3) = (R (*)(P0, P1, P2, P3))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3);
template<class C, class P0, class P1, class P2, class P3>
struct ClassBinder<C* (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3) = (C* (*)(P0, P1, P2, P3))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3) };
template<class C, class P0, class P1, class P2, class P3>
struct ClassBinder<void (C::*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3) };
template<class C, class R, class P0, class P1, class P2, class P3>
struct ClassBinder<R (C::*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3, P4);
template<class P0, class P1, class P2, class P3, class P4>
struct Binder<void (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4);
    void (*fp)(P0, P1, P2, P3, P4) = (void (*)(P0, P1, P2, P3, P4))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4);
template<class R, class P0, class P1, class P2, class P3, class P4>
struct Binder<R (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4);
    R (*fp)(P0, P1, P2, P3, P4) = (R (*)(P0, P1, P2, P3, P4))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4);
template<class C, class P0, class P1, class P2, class P3, class P4>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3, P4) = (C* (*)(P0, P1, P2, P3, P4))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4) };
template<class C, class P0, class P1, class P2, class P3, class P4>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3, P4);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3, P4);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3, P4, P5);
template<class P0, class P1, class P2, class P3, class P4, class P5>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5);
    void (*fp)(P0, P1, P2, P3, P4, P5) = (void (*)(P0, P1, P2, P3, P4, P5))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5);
    R (*fp)(P0, P1, P2, P3, P4, P5) = (R (*)(P0, P1, P2, P3, P4, P5))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3, P4, P5) = (C* (*)(P0, P1, P2, P3, P4, P5))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3, P4, P5, P6);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6) = (void (*)(P0, P1, P2, P3, P4, P5, P6))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6) = (R (*)(P0, P1, P2, P3, P4, P5, P6))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3, P4, P5, P6) = (C* (*)(P0, P1, P2, P3, P4, P5, P6))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3, P4, P5, P6, P7);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8); (void)(narg); CHECK(9);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))func_ptr;
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, void* func_ptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8); (void)(narg); CHECK(9);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))func_ptr;
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8); (void)(narg); CHECK(9);
    if(new_func_ptr){
        C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))new_func_ptr;
        C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
        DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8); (void)(narg); CHECK(9);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)method_pptr;
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, mrb_value self, void* method_pptr, mrb_value* args, int narg) {
    (void)(narg); CHECK(0); (void)(narg); CHECK(1); (void)(narg); CHECK(2); (void)(narg); CHECK(3); (void)(narg); CHECK(4); (void)(narg); CHECK(5); (void)(narg); CHECK(6); (void)(narg); CHECK(7); (void)(narg); CHECK(8); (void)(narg); CHECK(9);
    C* instance = static_cast<C*>(DATA_PTR(self));
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)method_pptr;
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return Type<R>::ret(mrb, result);
  }
};

#undef ARG
#undef CHECK

}  // namespace mrubybind

#undef ASSERT

namespace mrubybind {

//===========================================================================
// MrubyBind - utility class for binding C functions/classes to mruby.
class MrubyBind {
public:
  MrubyBind(mrb_state* mrb);
  MrubyBind(mrb_state* mrb, RClass* mod);
  ~MrubyBind();

  // Bind constant value.
  template <class T>
  void bind_const(const char* name, T v) {
    mrb_define_const(mrb_, mod_, name, Type<T>::ret(mrb_, v));
  }

  // Bind function.
  template <class Func>
  void bind(const char* func_name, Func func_ptr) {
    mrb_value mod = mrb_obj_value(mod_);
    mrb_value binder = mrb_voidp_value(mrb_, (void*)Binder<Func>::call);
    mrb_value func_name_v = mrb_str_new_cstr(mrb_, func_name);
    mrb_value func_ptr_v = mrb_voidp_value(mrb_, reinterpret_cast<void*>(func_ptr));
    mrb_value nparam_v = mrb_fixnum_value(Binder<Func>::NPARAM);
    mrb_funcall(mrb_, mod_mrubybind_, "define_function", 5, mod, binder, func_name_v,
                func_ptr_v, nparam_v);
  }

  // Bind class.
  template <class Func>
  void bind_class(const char* class_name, Func new_func_ptr) {
    struct RClass *tc = mrb_define_class(mrb_, class_name, mrb_->object_class);
    MRB_SET_INSTANCE_TT(tc, MRB_TT_DATA);
    mrb_value mod = mrb_obj_value(mod_);
    mrb_value binder = mrb_voidp_value(mrb_, (void*)ClassBinder<Func>::ctor);
    mrb_value class_name_v = mrb_str_new_cstr(mrb_, class_name);
    mrb_value new_func_ptr_v = mrb_voidp_value(mrb_, (void*)new_func_ptr);
    mrb_value nparam_v = mrb_fixnum_value(ClassBinder<Func>::NPARAM);
    mrb_funcall(mrb_, mod_mrubybind_, "bind_class", 5, mod, binder,
                class_name_v, new_func_ptr_v, nparam_v);
  }
  
  // Bind class.(no new func)
  template <class C>
  void bind_class(const char* module_name, const char* class_name) {
    struct RClass *tc = mrb_define_class(mrb_, class_name, mrb_->object_class);
    MRB_SET_INSTANCE_TT(tc, MRB_TT_DATA);
    mrb_value mod = mrb_obj_value(mod_);
    if(module_name){
        mod = mrb_obj_value(mrb_define_module(mrb_, module_name));
    }
    mrb_value binder = mrb_voidp_value(mrb_, (void*)ClassBinder<C*(*)(void)>::ctor);
    mrb_value class_name_v = mrb_str_new_cstr(mrb_, class_name);
    mrb_value new_func_ptr_v = mrb_nil_value();
    mrb_value nparam_v = mrb_fixnum_value(0);
    Type<C>::class_name = class_name;
    mrb_funcall(mrb_, mod_mrubybind_, "bind_class", 5, mod, binder,
                class_name_v, new_func_ptr_v, nparam_v);
  }
  
  template <class C>
  void bind_class(const char* class_name) {
    bind_class<C>(NULL, class_name);
  }

  // Bind instance method.
  template <class Method>
  void bind_instance_method(const char* class_name, const char* method_name,
                            Method method_ptr) {
    mrb_value mod = mrb_obj_value(mod_);
    mrb_value binder = mrb_voidp_value(mrb_, (void*)ClassBinder<Method>::call);
    mrb_value class_name_v = mrb_str_new_cstr(mrb_, class_name);
    mrb_value method_name_v = mrb_str_new_cstr(mrb_, method_name);
    mrb_value method_pptr_v = mrb_str_new(mrb_,
                                          reinterpret_cast<char*>(&method_ptr),
                                          sizeof(method_ptr));
    mrb_value nparam_v = mrb_fixnum_value(ClassBinder<Method>::NPARAM);
    mrb_funcall(mrb_, mod_mrubybind_, "bind_instance_method", 6,
                mod, binder, class_name_v, method_name_v, method_pptr_v, nparam_v);
  }

  // Bind static method.
  template <class Method>
  void bind_static_method(const char* class_name, const char* method_name,
                          Method method_ptr) {
    mrb_value mod = mrb_obj_value(mod_);
    mrb_value binder = mrb_voidp_value(mrb_, (void*)Binder<Method>::call);
    mrb_value class_name_v = mrb_str_new_cstr(mrb_, class_name);
    mrb_value method_name_v = mrb_str_new_cstr(mrb_, method_name);
    mrb_value func_ptr_v = mrb_voidp_value(mrb_, reinterpret_cast<void*>(method_ptr));
    mrb_value nparam_v = mrb_fixnum_value(Binder<Method>::NPARAM);
    mrb_funcall(mrb_, mod_mrubybind_, "bind_static_method", 6,
                mod, binder, class_name_v, method_name_v, func_ptr_v, nparam_v);
  }
  
  // Bind custom method.
  template <class Func>
  void bind_custom_method(const char* module_name, const char* class_name, const char* method_name, Func func_ptr) {
    mrb_value mod = mrb_obj_value(mod_);
    if(module_name){
        mod = mrb_obj_value(mrb_define_module(mrb_, module_name));
    }
    mrb_value binder = mrb_voidp_value(mrb_, (void*)Binder<Func>::call);
    mrb_value class_name_v = mrb_str_new_cstr(mrb_, class_name);
    mrb_value method_name_v = mrb_str_new_cstr(mrb_, method_name);
    mrb_value func_ptr_v = mrb_voidp_value(mrb_, reinterpret_cast<void*>(func_ptr));
    mrb_value nparam_v = mrb_fixnum_value(Binder<Func>::NPARAM - 1);
    mrb_funcall(mrb_, mod_mrubybind_, "bind_custom_method", 6,
                mod, binder, class_name_v, method_name_v, func_ptr_v, nparam_v);
  }
  
  template <class Func>
  void bind_custom_method(const char* class_name, const char* method_name, Func func_ptr) {
    bind_custom_method(NULL, class_name, method_name, func_ptr);
  }

  mrb_state* get_mrb(){
      return mrb_;
  }
  mrb_value get_avoid_gc_table(){
      return avoid_gc_table_;
  }

private:
  void Initialize();

  mrb_state* mrb_;
  mrb_value mod_mrubybind_;
  RClass* mod_;
  mrb_value avoid_gc_table_;
  int arena_index_;
};

MrubyRef load_string(mrb_state* mrb, std::string code);

}  // namespace mrubybind

#endif
