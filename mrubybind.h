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
#include "mruby/proc.h"
#include "mruby/variable.h"
//#include "mrubybind_types.h"

#include <iostream>
#include <vector>
// Describe type conversion between C type value and mruby value.

#include "mruby/string.h"
#include "mruby/proc.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/variable.h"
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <iostream>

namespace mrubybind {

extern const char* untouchable_table;
extern const char* untouchable_object;

//================================================================//
// MrubyArenaStore
//================================================================//
class MrubyArenaStore {

	mrb_state* mrb;
	int ai;
public:
	MrubyArenaStore ( mrb_state* mrb ) {
		this->mrb = mrb;
		this->ai = mrb_gc_arena_save ( mrb );
	}

	~MrubyArenaStore () {
		mrb_gc_arena_restore ( this->mrb, this->ai );
	}

};

//================================================================//
// MrubyBindStatus
//================================================================//
class MrubyBindStatus {

public:

	struct Data;
	typedef std::shared_ptr< Data > Data_ptr;
	typedef std::map< mrb_state*, Data_ptr > Table;

	struct ObjectInfo {

		size_t ref_count;
		size_t id;

		ObjectInfo () {
			this->ref_count = 0;
			this->id = 0;
		}

		ObjectInfo ( size_t id ) {
			this->ref_count = 1;
			this->id = id;
		}
	};
	typedef std::map< RBasic*, ObjectInfo > ObjectIdTable;
	typedef std::vector< size_t > FreeIdArray;

	static Table& get_living_table () {
		static Table table;
		return table;
	}

	struct Data {

		typedef std::map< std::string, std::map< std::string, bool > > ClassConvertableTable;


		mrb_state* mrb;
		mrb_value avoid_gc_table;
		ClassConvertableTable class_convertable_table;
		ObjectIdTable object_id_table;
		FreeIdArray free_id_array;

		Data () {}
		~Data () {}

		mrb_state* get_mrb () {
			return mrb;
		}

		mrb_value get_avoid_gc_table () {
			return avoid_gc_table;
		}

		size_t new_id () {
			//return mrb_ary_len(mrb, avoid_gc_table);
			return RARRAY_LEN ( avoid_gc_table );
		}

		ObjectIdTable& get_object_id_table () {
			return object_id_table;
		}

		FreeIdArray& get_free_id_array () {
			return free_id_array;
		}

		void set_class_conversion ( const std::string& s, const std::string& d, bool c ) {
			class_convertable_table [ s ][ d ] = c;
		}

		bool is_convertable ( const std::string& s, const std::string& d ) {
			auto fs = class_convertable_table.find ( s );
			if ( fs != class_convertable_table.end () ) {
				auto fd = fs->second.find ( d );
				if ( fd != fs->second.end () ) {
					return fd->second;
				}
			}
			return false;
		}

		bool add_avoid_gc_object ( const mrb_value& v ) {

			if ( !mrb_immediate_p ( v ) ) {

				auto& mrbsp = MrubyBindStatus::search ( mrb );
				mrb_value avoid_gc_table = mrbsp->get_avoid_gc_table ();
				auto& object_id_table = mrbsp->get_object_id_table ();
				auto& free_id_array = mrbsp->get_free_id_array ();

				if ( object_id_table.find ( mrb_basic_ptr ( v ) ) == object_id_table.end () ) {

					size_t new_id;
					if ( !free_id_array.empty () ) {

						new_id = free_id_array.back ();
						free_id_array.pop_back ();

						mrb_ary_set ( mrb, avoid_gc_table, ( mrb_int )new_id, v );
					}
					else {
						new_id = mrbsp->new_id ();
						mrb_ary_push ( mrb, avoid_gc_table, v );
					}

					object_id_table [ mrb_basic_ptr ( v ) ] = MrubyBindStatus::ObjectInfo ( new_id );
				}
				else {
					object_id_table [ mrb_basic_ptr ( v ) ].ref_count++;
				}

				return true;
			}

			return false;
		}

		bool reduce_avoid_gc_object ( const mrb_value& v ) {

			if ( !mrb_immediate_p ( v ) ) {

				auto& mrbsp = MrubyBindStatus::search ( mrb );
				mrb_value avoid_gc_table = mrbsp->get_avoid_gc_table ();
				auto& object_id_table = mrbsp->get_object_id_table ();
				auto& free_id_array = mrbsp->get_free_id_array ();

				if ( object_id_table.find ( mrb_basic_ptr ( v ) ) != object_id_table.end () ) {

					auto& obj_info = object_id_table [ mrb_basic_ptr ( v ) ];
					obj_info.ref_count--;

					if ( obj_info.ref_count <= 0 ) {

						mrb_ary_set ( mrb, avoid_gc_table, ( mrb_int )obj_info.id, mrb_nil_value () );
						free_id_array.push_back ( obj_info.id );
						object_id_table.erase ( mrb_basic_ptr ( v ) );
					}

					return true;
				}

				return false;
			}

			return false;
		}

	};

	MrubyBindStatus () {
	}

	MrubyBindStatus ( mrb_state* mrb, mrb_value avoid_gc_table ) {

		Table& living_table = MrubyBindStatus::get_living_table ();
		data = std::make_shared < Data > ();
		data->mrb = mrb;
		data->avoid_gc_table = avoid_gc_table;
		living_table [ mrb ] = data;
	}

	~MrubyBindStatus () {

		Table& living_table = MrubyBindStatus::get_living_table ();
		living_table.erase ( data->mrb );
		data->mrb = NULL;
	}

	static bool is_living ( mrb_state* mrb ) {

		Table& living_table = get_living_table ();
		if ( living_table.find ( mrb ) != living_table.end () ) {
			return living_table [ mrb ].get ();
		}
		return false;
	}

	static Data_ptr search ( mrb_state* mrb ) {

		Table& living_table = MrubyBindStatus::get_living_table ();
		if ( living_table.find ( mrb ) != living_table.end () ) {
			return living_table [ mrb ];
		}
		return Data_ptr ( NULL );
	}

private:
	std::shared_ptr< Data > data;

};

//================================================================//
// NullDeleter<T>
//================================================================//
template<class T>
struct NullDeleter {
	void operator()( T* p ) const {}
};

struct GCDeleter {
	mrb_state* mrb;
	GCDeleter ( mrb_state* mrb ) { this->mrb = mrb; }
	void operator()( RBasic* p ) const { mrb_gc_unregister ( mrb, mrb_obj_value(p) ); }
};

//================================================================//
// Deleter<T>
//================================================================//
template<class T>
class Deleter {
private:
    MrubyBindStatus::Data_ptr mrbsp;
    mrb_value v_;
public:
    Deleter() { }

	Deleter ( mrb_state* mrb, mrb_value v ) {

		if ( !mrb_immediate_p ( v ) ) {

			mrbsp = MrubyBindStatus::search ( mrb );
			mrbsp->add_avoid_gc_object ( v );
			/*mrb_value avoid_gc_table = mrbsp->get_avoid_gc_table ();
			auto& object_id_table = mrbsp->get_object_id_table ();
			auto& free_id_array = mrbsp->get_free_id_array ();

			if ( object_id_table.find ( mrb_basic_ptr ( v ) ) == object_id_table.end () ) {

				size_t new_id;
				if ( !free_id_array.empty () ) {

					new_id = free_id_array.back ();
					free_id_array.pop_back ();

					mrb_ary_set ( mrb, avoid_gc_table, ( mrb_int )new_id, v );
				}
				else {
					new_id = mrbsp->new_id ();
					mrb_ary_push ( mrb, avoid_gc_table, v );
				}

				object_id_table [ mrb_basic_ptr ( v ) ] = MrubyBindStatus::ObjectInfo ( new_id );
			}
			else {
				object_id_table [ mrb_basic_ptr ( v ) ].ref_count++;
			}*/
		}
		v_ = v;

	}

    ~Deleter() { }

	mrb_state* get_mrb () {
		return mrbsp->mrb;
	}

	void operator()( T* p ) const {

		if ( mrbsp.get () ) {

			mrb_state* mrb = mrbsp->get_mrb ();
			if ( mrb ) {
				
				mrbsp->reduce_avoid_gc_object ( v_ );
				/*mrb_value v = v_;
				if ( !mrb_immediate_p ( v ) ) {

					mrb_value avoid_gc_table = mrbsp->get_avoid_gc_table ();
					auto& object_id_table = mrbsp->get_object_id_table ();
					auto& free_id_array = mrbsp->get_free_id_array ();
					auto& oi = object_id_table [ mrb_basic_ptr ( v ) ];
					oi.ref_count--;

					if ( oi.ref_count <= 0 ) {
						mrb_ary_set ( mrb, avoid_gc_table, ( mrb_int )oi.id, mrb_nil_value () );
						free_id_array.push_back ( oi.id );
						object_id_table.erase ( mrb_basic_ptr ( v ) );
					}
				}*/
			}
		}
		if ( p ) {
			delete p;
		}
	}

};

template<class T> using obj_ptr = std::shared_ptr<T>;
//template<class T> using FuncPtr = std::shared_ptr<std::function<T> >;

//================================================================//
// FuncPtr<T>
//================================================================//
template<class T>
class FuncPtr {
private:
	mrb_state* mrb;
	std::shared_ptr< std::function<T> > p;
public:
	FuncPtr () {
	}
	template<class D>FuncPtr ( std::function<T>* pt, D d ) : p ( pt, d ) {
		mrb = d.get_mrb ();
	}
	~FuncPtr () {

	}
	const std::shared_ptr<std::function<T> >& ref () const {
		return p;
	}
	bool is_living () const {
		return MrubyBindStatus::is_living ( mrb );
	}
	std::function<T>& func () const {
		if ( !p.get () ) {
			throw std::runtime_error ( "empty function." );
		}
		return *p.get ();
	}
	operator bool () const {
		if ( !p.get () ) {
			return false;
		}
		return ( bool )*p.get ();
	}
	void reset () {
		p.reset ();
	}
	template<class Y> void reset ( Y* y ) {
		p.reset ( y );
	}
	template<class Y, class D> void reset ( Y* y, D d ) {
		p.reset ( y, d );
	}
	template<class Y, class D, class A> void reset ( Y* y, D d, A a ) {
		p.reset ( y, d, a );
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

//================================================================//
// MrubyRef
//================================================================//
class MrubyRef {
protected:

	mrb_state* mrb;
	std::shared_ptr<mrb_value> v;
	std::shared_ptr<RObject> strong_v;
	std::weak_ptr<RObject> weak_v;

public:

	enum {
		MAKE_STRONG,
		MAKE_WEAK,
	};

	friend class MrubyStrongRef;
	friend class MrubyWeakRef;

	MrubyRef ();
	MrubyRef ( mrb_state* mrb, const mrb_value& v );
	MrubyRef ( const MrubyRef& assign );
	~MrubyRef ();

	void					clear ();
	void					make_strong ( mrb_state* mrb, const mrb_value& v );
	void					make_weak ( mrb_state* mrb, const mrb_value& v );
	virtual void			set_ref ( mrb_state* mrb, const mrb_value& v );
	void					set_ref ( mrb_state* mrb, const mrb_value& v, unsigned int type );
	void					take ( const MrubyRef& assign );

	bool is_living () const;
	mrb_state* get_mrb () const;
	virtual mrb_value get_v () const;
	virtual bool empty () const;
	bool test () const;
	bool obj_equal ( const MrubyRef& r ) const;
	std::string to_s () const;
	int to_i () const;
	float to_float () const;
	double to_double () const;

	//----------------------------------------------------------------//
	/*inline mrb_value* operator -> () const {
		return &this->get_v ();
	};*/

	//----------------------------------------------------------------//
	/*inline mrb_value& operator * () const {
		return this->get_v ();
	};*/

	//----------------------------------------------------------------//
	/*inline operator mrb_value* () {
		return &this->get_v ();
	};*/

    MrubyRef call(std::string name);

#include "mrubybind_call_generated.h"

};

//================================================================//
// MrubyStrongRef
//================================================================//
class MrubyStrongRef :
	public MrubyRef {
public:

	//----------------------------------------------------------------//
					MrubyStrongRef ();
					MrubyStrongRef ( mrb_state* mrb, const mrb_value& v );
					MrubyStrongRef ( const MrubyStrongRef& assign );
	void			set_ref ( mrb_state* mrb, const mrb_value& v );
	mrb_value		get_v () const;
	bool			empty () const;

	//----------------------------------------------------------------//
	inline operator bool () const {
		return !this->empty ();
	};

	//----------------------------------------------------------------//
	inline operator mrb_value() const {
		return this->get_v ();
	};

	//----------------------------------------------------------------//
	inline MrubyStrongRef& operator = ( const MrubyStrongRef& rhs ) {

		if ( this == &rhs ) {
			return *this;
		}

		this->mrb = rhs.mrb;
		this->strong_v = rhs.strong_v;

		return *this;
	};
};

//================================================================//
// MrubyStrongRef
//================================================================//
class MrubyWeakRef :
	public MrubyRef {
public:

	//----------------------------------------------------------------//
					MrubyWeakRef ();
					MrubyWeakRef ( mrb_state* mrb, const mrb_value& v );
					MrubyWeakRef ( const MrubyWeakRef& assign );
	void			set_ref ( mrb_state* mrb, const mrb_value& v );
	mrb_value		get_v () const;
	bool			empty () const;

	//----------------------------------------------------------------//
	inline operator bool() const {
		return !this->empty ();
	};

	//----------------------------------------------------------------//
	inline operator mrb_value() const {
		return this->get_v ();
	};

	//----------------------------------------------------------------//
	inline MrubyWeakRef& operator = ( const MrubyWeakRef& rhs ) {

		if ( this == &rhs ) {
			return *this;
		}

		this->mrb = rhs.mrb;
		this->weak_v = rhs.weak_v;

		return *this;
	};
};


//===========================================================================
// C <-> mruby type converter.

//template <class T>
//struct Type {
  // Type name used for error message.
  // static const char TYPE_NAME[];

  // Returns whether the given mrb_value can be converted into type T.
  //static int check(mrb_value v) = 0;

  // Converts mrb_value to type T value.
  //static T get(mrb_value v) = 0;

  // Converts type T value to mrb_value.
  //static mrb_value ret(mrb_state*, T i) = 0;
//};

// Fixnum
template<>
struct Type<int> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_fixnum_p(v) || mrb_float_p(v); }
  static int get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_fixnum_p(v) ? mrb_fixnum(v) : mrb_float(v); }
  static mrb_value ret(mrb_state*, int i) { return mrb_fixnum_value(i); }
};

template<>
struct Type<unsigned int> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_fixnum_p(v) || mrb_float_p(v); }
  static unsigned int get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_fixnum_p(v) ? mrb_fixnum(v) : mrb_float(v); }
  static mrb_value ret(mrb_state*, unsigned int i) { return mrb_fixnum_value(i); }
};

// float
template<>
struct Type<float> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_float_p(v) || mrb_fixnum_p(v); }
  static float get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_float_p(v) ? mrb_float(v) : mrb_fixnum(v); }
  static mrb_value ret(mrb_state* mrb, float f) { return mrb_float_value(mrb, f); }
};

// double
template<>
struct Type<double> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_float_p(v) || mrb_fixnum_p(v); }
  static double get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_float_p(v) ? mrb_float(v) : mrb_fixnum(v); }
  static mrb_value ret(mrb_state* mrb, double f) { return mrb_float_value(mrb, f); }
};

// String
template<>
struct Type<const char*> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_string_p(v); }
  static const char* get(mrb_state* mrb, mrb_value v) { (void)mrb; return RSTRING_PTR(v); }
  static mrb_value ret(mrb_state* mrb, const char* s) { return mrb_str_new_cstr(mrb, s); }
};

template<>
struct Type<std::string> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_string_p(v); }
  static const std::string get(mrb_state* mrb, mrb_value v) { (void)mrb; return std::string(RSTRING_PTR(v), RSTRING_LEN(v)); }
  static mrb_value ret(mrb_state* mrb, const std::string& s) { return mrb_str_new(mrb, s.c_str(), s.size()); }
};

template<>
struct Type<const std::string> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_string_p(v); }
  static const std::string get(mrb_state* mrb, mrb_value v) { (void)mrb; return std::string(RSTRING_PTR(v), RSTRING_LEN(v)); }
  static mrb_value ret(mrb_state* mrb, const std::string& s) { return mrb_str_new(mrb, s.c_str(), s.size()); }
};

template<>
struct Type<const std::string&> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_string_p(v); }
  static const std::string get(mrb_state* mrb, mrb_value v) { (void)mrb; return std::string(RSTRING_PTR(v), RSTRING_LEN(v)); }
  static mrb_value ret(mrb_state* mrb, const std::string& s) { return mrb_str_new(mrb, s.c_str(), s.size()); }
};

// Boolean
template<>
struct Type<bool> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value /*v*/) { return 1; }
  static bool get(mrb_state* mrb, mrb_value v) { (void)mrb; return mrb_test(v); }
  static mrb_value ret(mrb_state* /*mrb*/, bool b) { return b ? mrb_true_value() : mrb_false_value(); }
};

// Raw pointer
template<>
struct Type<void*> {
  static const char TYPE_NAME[];
  static int check(mrb_state*, mrb_value v) { return mrb_cptr_p(v); }
  static void* get(mrb_state*, mrb_value v) { return mrb_cptr(v); }
  static mrb_value ret(mrb_state* mrb, void* p) { printf("Type<void*>::ret()"); return mrb_cptr_value(mrb, p); }
};

// Function
struct TypeFuncBase{
    static const char TYPE_NAME[];
};

template<class R>
struct Type<FuncPtr<R()> > :public TypeFuncBase {
  static int check(mrb_state*, mrb_value v) { return mrb_type(v) == MRB_TT_PROC; }
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
  static int check(mrb_state*, mrb_value v) { return mrb_type(v) == MRB_TT_PROC; }
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
	static int check ( mrb_state*, mrb_value ) { return 1; }
	static MrubyRef get ( mrb_state* mrb, mrb_value v ) { ( void )mrb; return MrubyRef ( mrb, v ); }
	static mrb_value ret ( mrb_state*, MrubyRef r ) { return r.get_v (); }
};

// mruby StrongRef
template<>
struct Type<MrubyStrongRef> {
	static const char TYPE_NAME[];
	static int check ( mrb_state*, mrb_value ) { return 1; }
	static MrubyStrongRef get ( mrb_state* mrb, mrb_value v ) { ( void )mrb; return MrubyStrongRef ( mrb, v ); }
	static mrb_value ret ( mrb_state*, MrubyStrongRef r ) { return r.get_v (); }
};

// mruby WeakRef
template<>
struct Type<MrubyWeakRef> {
	static const char TYPE_NAME[];
	static int check ( mrb_state*, mrb_value ) { return 1; }
	static MrubyWeakRef get ( mrb_state* mrb, mrb_value v ) { ( void )mrb; return MrubyWeakRef ( mrb, v ); }
	static mrb_value ret ( mrb_state*, MrubyWeakRef r ) { return r; }
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
	  C* instance = static_cast< C* >(p);
	  delete instance;
  }

  // Template specialization.
  //static void ctor(mrb_state* mrb, mrb_value self, void* new_func_ptr, mrb_value* args, int narg) {
};

template<class C>
mrb_data_type ClassBinder<C>::type_info = { "???", dtor };

template <class T>
struct CustomClassBinder {
  // Template specialization.
  //static mrb_value call(mrb_state* mrb, void* p, mrb_value* args, int narg) = 0;
};

// Other Class
struct TypeClassBase{
    static const char TYPE_NAME[];
};

template<class T> struct Type<T&> :public TypeClassBase {
    static std::string class_name;
    static int check(mrb_state* mrb, mrb_value v) {
        return mrb_type(v) == MRB_TT_DATA &&
            MrubyBindStatus::search(mrb)->is_convertable(mrb_obj_classname(mrb, v), class_name);
    }
    static T& get(mrb_state* mrb, mrb_value v) {
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

template<class T> std::string Type<T&>::class_name = "";

template<class T> struct Type : public TypeClassBase {

    static std::string class_name;

    static int check(mrb_state* mrb, mrb_value v) {
        return mrb_type(v) == MRB_TT_DATA &&
            MrubyBindStatus::search(mrb)->is_convertable(mrb_obj_classname(mrb, v), class_name);
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
mrb_value raisenarg(mrb_state *mrb, mrb_value func_name, int narg, int nparam);

// Includes generated template specialization.
//#include "mrubybind.inc"
// This file is generated from gen_template.rb
#define ARG(mrb, i)  Type<P##i>::get(mrb, args[i])
#define ARGSHIFT(mrb, i, j)  Type<P##i>::get(mrb, args[j])
#define CHECK(i)  {if(!Type<P##i>::check(mrb, args[i])) return RAISE(i);}
#define CHECKSHIFT(i, j)  {if(!Type<P##i>::check(mrb, args[j])) return RAISE(j);}
#define RAISE(i)  raise(mrb, i, Type<P##i>::TYPE_NAME, args[i])
#define CHECKNARG(narg)  {if(narg != NPARAM) RAISENARG(narg);}
#define RAISENARG(narg)  raisenarg(mrb, mrb_cfunc_env_get(mrb, 1), narg, NPARAM)

// void f(void);
template<>
struct Binder<void (*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(void) = (void (*)(void))mrb_cptr(cfunc);
    fp();
    return mrb_nil_value();
  }
};

// R f(void);
template<class R>
struct Binder<R (*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(void) = (R (*)(void))mrb_cptr(cfunc);
    R result = fp();
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(void);
template<class C>
struct ClassBinder<C* (*)(void)> {
  static const int NPARAM = 0;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(void) = (C* (*)(void))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(void);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)();
    return mrb_nil_value();
  }
};

// class C { R f(void) };
template<class C, class R>
struct ClassBinder<R (C::*)(void)> {
  static const int NPARAM = 0;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(void);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)();
    return Type<R>::ret(mrb, result);
  }
};

// void f(P0);
template<class P0>
struct Binder<void (*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0) = (void (*)(P0))mrb_cptr(cfunc);
    fp(ARG(mrb, 0));
    return mrb_nil_value();
  }
};

// R f(P0);
template<class R, class P0>
struct Binder<R (*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0) = (R (*)(P0))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0);
template<class C, class P0>
struct ClassBinder<C* (*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0) = (C* (*)(P0))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0));
    return mrb_nil_value();
  }
};

// class C { R f(P0) };
template<class C, class R, class P0>
struct ClassBinder<R (C::*)(P0)> {
  static const int NPARAM = 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0>
struct CustomClassBinder<void (*)(P0)> {
  static const int NPARAM = 1 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance);
    return mrb_nil_value();
  }
};

template<class R, class P0>
struct CustomClassBinder<R (*)(P0)> {
  static const int NPARAM = 1 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance);
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0>
struct CustomClassBinder<void (*)(P0&)> {
  static const int NPARAM = 1 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance);
    return mrb_nil_value();
  }
};

template<class R, class P0>
struct CustomClassBinder<R (*)(P0&)> {
  static const int NPARAM = 1 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance);
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1);
template<class P0, class P1>
struct Binder<void (*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1) = (void (*)(P0, P1))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1));
    return mrb_nil_value();
  }
};

// R f(P0, P1);
template<class R, class P0, class P1>
struct Binder<R (*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1) = (R (*)(P0, P1))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1);
template<class C, class P0, class P1>
struct ClassBinder<C* (*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1) = (C* (*)(P0, P1))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1) };
template<class C, class R, class P0, class P1>
struct ClassBinder<R (C::*)(P0, P1)> {
  static const int NPARAM = 2;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1>
struct CustomClassBinder<void (*)(P0, P1)> {
  static const int NPARAM = 2 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1>
struct CustomClassBinder<R (*)(P0, P1)> {
  static const int NPARAM = 2 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1>
struct CustomClassBinder<void (*)(P0&, P1)> {
  static const int NPARAM = 2 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1>
struct CustomClassBinder<R (*)(P0&, P1)> {
  static const int NPARAM = 2 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2);
template<class P0, class P1, class P2>
struct Binder<void (*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2) = (void (*)(P0, P1, P2))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2);
template<class R, class P0, class P1, class P2>
struct Binder<R (*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2) = (R (*)(P0, P1, P2))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2);
template<class C, class P0, class P1, class P2>
struct ClassBinder<C* (*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2) = (C* (*)(P0, P1, P2))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2) };
template<class C, class R, class P0, class P1, class P2>
struct ClassBinder<R (C::*)(P0, P1, P2)> {
  static const int NPARAM = 3;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2>
struct CustomClassBinder<void (*)(P0, P1, P2)> {
  static const int NPARAM = 3 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2>
struct CustomClassBinder<R (*)(P0, P1, P2)> {
  static const int NPARAM = 3 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2>
struct CustomClassBinder<void (*)(P0&, P1, P2)> {
  static const int NPARAM = 3 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2>
struct CustomClassBinder<R (*)(P0&, P1, P2)> {
  static const int NPARAM = 3 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3);
template<class P0, class P1, class P2, class P3>
struct Binder<void (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3) = (void (*)(P0, P1, P2, P3))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3);
template<class R, class P0, class P1, class P2, class P3>
struct Binder<R (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3) = (R (*)(P0, P1, P2, P3))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3);
template<class C, class P0, class P1, class P2, class P3>
struct ClassBinder<C* (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3) = (C* (*)(P0, P1, P2, P3))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3) };
template<class C, class R, class P0, class P1, class P2, class P3>
struct ClassBinder<R (C::*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3>
struct CustomClassBinder<void (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3>
struct CustomClassBinder<R (*)(P0, P1, P2, P3)> {
  static const int NPARAM = 4 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3)> {
  static const int NPARAM = 4 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3)> {
  static const int NPARAM = 4 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4);
template<class P0, class P1, class P2, class P3, class P4>
struct Binder<void (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4) = (void (*)(P0, P1, P2, P3, P4))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4);
template<class R, class P0, class P1, class P2, class P3, class P4>
struct Binder<R (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4) = (R (*)(P0, P1, P2, P3, P4))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4);
template<class C, class P0, class P1, class P2, class P3, class P4>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4) = (C* (*)(P0, P1, P2, P3, P4))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4)> {
  static const int NPARAM = 5 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4)> {
  static const int NPARAM = 5 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4)> {
  static const int NPARAM = 5 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5);
template<class P0, class P1, class P2, class P3, class P4, class P5>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5) = (void (*)(P0, P1, P2, P3, P4, P5))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5) = (R (*)(P0, P1, P2, P3, P4, P5))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5) = (C* (*)(P0, P1, P2, P3, P4, P5))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5)> {
  static const int NPARAM = 6 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6) = (void (*)(P0, P1, P2, P3, P4, P5, P6))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6) = (R (*)(P0, P1, P2, P3, P4, P5, P6))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6) = (C* (*)(P0, P1, P2, P3, P4, P5, P6))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6)> {
  static const int NPARAM = 7 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7)> {
  static const int NPARAM = 8 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8)> {
  static const int NPARAM = 9 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))mrb_cptr(cfunc);
    if(ctor)
    {
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
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9)> {
  static const int NPARAM = 10 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> {
  static const int NPARAM = 11 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11)> {
  static const int NPARAM = 12 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12)> {
  static const int NPARAM = 13 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13)> {
  static const int NPARAM = 14 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14)> {
  static const int NPARAM = 15 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15)> {
  static const int NPARAM = 16 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14));
    return Type<R>::ret(mrb, result);
  }
};


// void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct Binder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15); CHECK(16);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    void (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) = (void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16))mrb_cptr(cfunc);
    fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15), ARG(mrb, 16));
    return mrb_nil_value();
  }
};

// R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct Binder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17;
  static mrb_value call(mrb_state* mrb, mrb_value /*self*/) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15); CHECK(16);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    R (*fp)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) = (R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16))mrb_cptr(cfunc);
    R result = fp(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15), ARG(mrb, 16));
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct ClassBinder<C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17;
  static mrb_value ctor(mrb_state* mrb, mrb_value self) {
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = NULL;
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15); CHECK(16);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    C* (*ctor)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) = (C* (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16))mrb_cptr(cfunc);
    if(ctor)
    {
      C* instance = ctor(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15), ARG(mrb, 16));
      DATA_PTR(self) = instance;
    }
    return self;
  }
};

// class C { void f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) };
template<class C, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct ClassBinder<void (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15); CHECK(16);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
    M mp = *(M*)RSTRING_PTR(cmethod);
    (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15), ARG(mrb, 16));
    return mrb_nil_value();
  }
};

// class C { R f(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) };
template<class C, class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct ClassBinder<R (C::*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECK(0); CHECK(1); CHECK(2); CHECK(3); CHECK(4); CHECK(5); CHECK(6); CHECK(7); CHECK(8); CHECK(9); CHECK(10); CHECK(11); CHECK(12); CHECK(13); CHECK(14); CHECK(15); CHECK(16);
    C* instance = static_cast<C*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (C::*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = (instance->*mp)(ARG(mrb, 0), ARG(mrb, 1), ARG(mrb, 2), ARG(mrb, 3), ARG(mrb, 4), ARG(mrb, 5), ARG(mrb, 6), ARG(mrb, 7), ARG(mrb, 8), ARG(mrb, 9), ARG(mrb, 10), ARG(mrb, 11), ARG(mrb, 12), ARG(mrb, 13), ARG(mrb, 14), ARG(mrb, 15), ARG(mrb, 16));
    return Type<R>::ret(mrb, result);
  }
};


// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct CustomClassBinder<void (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14); CHECKSHIFT(16, 15);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14), ARGSHIFT(mrb, 16, 15));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct CustomClassBinder<R (*)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14); CHECKSHIFT(16, 15);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14), ARGSHIFT(mrb, 16, 15));
    return Type<R>::ret(mrb, result);
  }
};



// custom method
template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct CustomClassBinder<void (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14); CHECKSHIFT(16, 15);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef void (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
    M mp = *(M*)RSTRING_PTR(cmethod);
    mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14), ARGSHIFT(mrb, 16, 15));
    return mrb_nil_value();
  }
};

template<class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15, class P16>
struct CustomClassBinder<R (*)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16)> {
  static const int NPARAM = 17 - 1;
  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* targs;
    int narg;
    mrb_value block = mrb_nil_value();
    std::vector<mrb_value> args;
    mrb_get_args(mrb, "*|&", &targs, &narg, &block);
    args.resize(narg);
    if(narg > 0){
      ::memmove(&args[0], &targs[0], narg * sizeof(mrb_value));
    }
    if(mrb_test(block)){
      args.push_back(block);
      narg++;
    }
    CHECKNARG(narg); CHECKSHIFT(1, 0); CHECKSHIFT(2, 1); CHECKSHIFT(3, 2); CHECKSHIFT(4, 3); CHECKSHIFT(5, 4); CHECKSHIFT(6, 5); CHECKSHIFT(7, 6); CHECKSHIFT(8, 7); CHECKSHIFT(9, 8); CHECKSHIFT(10, 9); CHECKSHIFT(11, 10); CHECKSHIFT(12, 11); CHECKSHIFT(13, 12); CHECKSHIFT(14, 13); CHECKSHIFT(15, 14); CHECKSHIFT(16, 15);
    P0* instance = static_cast<P0*>(DATA_PTR(self));
    mrb_value cmethod = mrb_cfunc_env_get(mrb, 0);
    typedef R (*M)(P0&, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
    M mp = *(M*)RSTRING_PTR(cmethod);
    R result = mp(*instance, ARGSHIFT(mrb, 1, 0), ARGSHIFT(mrb, 2, 1), ARGSHIFT(mrb, 3, 2), ARGSHIFT(mrb, 4, 3), ARGSHIFT(mrb, 5, 4), ARGSHIFT(mrb, 6, 5), ARGSHIFT(mrb, 7, 6), ARGSHIFT(mrb, 8, 7), ARGSHIFT(mrb, 9, 8), ARGSHIFT(mrb, 10, 9), ARGSHIFT(mrb, 11, 10), ARGSHIFT(mrb, 12, 11), ARGSHIFT(mrb, 13, 12), ARGSHIFT(mrb, 14, 13), ARGSHIFT(mrb, 15, 14), ARGSHIFT(mrb, 16, 15));
    return Type<R>::ret(mrb, result);
  }
};


#undef ARG
#undef CHECK

}  // namespace mrubybind

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
    MrubyArenaStore store(mrb_);
    mrb_define_const(mrb_, mod_, name, Type<T>::ret(mrb_, v));
  }

  template <class T>
  void bind_const(const char* module_name, const char* class_name, const char* name, T v) {
    MrubyArenaStore store(mrb_);

    struct RClass * tc = DefineClass(module_name, class_name);

    mrb_define_const(mrb_, tc, name, Type<T>::ret(mrb_, v));
  }

  // Bind function.
  template <class Func>
  void bind(const char* func_name, Func func_ptr) {
    MrubyArenaStore store(mrb_);
    mrb_sym func_name_s = mrb_intern_cstr(mrb_, func_name);
    mrb_value env[] = {
      mrb_cptr_value(mrb_, (void*)func_ptr),  // 0: c function pointer
      mrb_symbol_value(func_name_s),          // 1: function name
    };
    struct RProc* proc = mrb_proc_new_cfunc_with_env(mrb_, Binder<Func>::call, 2, env);
    mrb_field_write_barrier(mrb_, (RBasic *)proc, (RBasic *)proc->e.env);
    mrb_method_t m;
    MRB_METHOD_FROM_PROC(m, proc);
    if (mod_ == mrb_->kernel_module)
      mrb_define_method_raw(mrb_, mod_, func_name_s, m);
    else
      mrb_define_class_method_raw(mrb_, mod_, func_name_s, proc);
  }

  // Bind class.
  template <class Func>
  void bind_class(const char* class_name, Func new_func_ptr) {
    MrubyArenaStore store(mrb_);
    struct RClass *tc = mrb_define_class(mrb_, class_name, mrb_->object_class);
    MRB_SET_INSTANCE_TT(tc, MRB_TT_DATA);
    BindInstanceMethod(class_name, "initialize",
                       mrb_cptr_value(mrb_, (void*)new_func_ptr),
                       ClassBinder<Func>::ctor);
  }

  // Bind class.(no new func)
  template <class C>
  void bind_class(const char* module_name, const char* class_name) {
    MrubyArenaStore store(mrb_);

    struct RClass * tc = DefineClass(module_name, class_name);
    std::string name;
    if(module_name){
        name += module_name;
        name += "::";
    }
    name += class_name;

    Type<C>::class_name = name;
    Type<C&>::class_name = name;
    MrubyBindStatus::search(mrb_)->set_class_conversion(name, name, true);
    MRB_SET_INSTANCE_TT(tc, MRB_TT_DATA);
    BindInstanceMethod(module_name, class_name, "initialize",
                       mrb_cptr_value(mrb_, NULL),
                       ClassBinder<C*(*)(void)>::ctor);
  }

  template <class C>
  void bind_class(const char* class_name) {
    bind_class<C>(NULL, class_name);
  }

  // Bind instance method.
  template <class Method>
  void bind_instance_method(const char* class_name, const char* method_name,
                            Method method_ptr) {
    MrubyArenaStore store(mrb_);
    mrb_value method_pptr_v = mrb_str_new(mrb_,
                                          reinterpret_cast<char*>(&method_ptr),
                                          sizeof(method_ptr));
    BindInstanceMethod(class_name, method_name,
                       method_pptr_v, ClassBinder<Method>::call);
  }

  // Bind static method.
  template <class Method>
  void bind_static_method(const char* module_name, const char* class_name, const char* method_name,
                          Method method_ptr) {
    MrubyArenaStore store(mrb_);
    mrb_sym method_name_s = mrb_intern_cstr(mrb_, method_name);
    mrb_value env[] = {
      mrb_cptr_value(mrb_, (void*)method_ptr),  // 0: method pointer
      mrb_symbol_value(method_name_s),          // 1: method name
    };
    struct RProc* proc = mrb_proc_new_cfunc_with_env(mrb_, Binder<Method>::call, 2, env);
    mrb_field_write_barrier(mrb_, (RBasic *)proc, (RBasic *)proc->e.env);
    struct RClass* klass = GetClass(module_name, class_name);
    mrb_define_class_method_raw(mrb_, klass, method_name_s, proc);
  }

  template <class Method>
  void bind_static_method(const char* class_name, const char* method_name,
                          Method method_ptr) {
    bind_static_method(NULL, class_name, method_name,
                          method_ptr);
  }

  // Bind custom method.
  template <class Func>
  void bind_custom_method(const char* module_name, const char* class_name, const char* method_name, Func func_ptr) {
    MrubyArenaStore store(mrb_);
    mrb_value (*binder_func)(mrb_state*, mrb_value) = CustomClassBinder<Func>::call;
    mrb_value original_func_v = mrb_str_new(mrb_,
                                          reinterpret_cast<char*>(&func_ptr),
                                          sizeof(func_ptr));
    mrb_sym method_name_s = mrb_intern_cstr(mrb_, method_name);
    mrb_value env[] = {
      original_func_v, // 0: c function pointer
      mrb_symbol_value(method_name_s), // 1: method name
    };
    struct RProc* proc = mrb_proc_new_cfunc_with_env(mrb_, binder_func, 2, env);
    mrb_field_write_barrier(mrb_, (RBasic *)proc, (RBasic *)proc->e.env);
    mrb_method_t m;
    MRB_METHOD_FROM_PROC(m, proc);
    struct RClass* klass = GetClass(module_name, class_name);
    mrb_define_method_raw(mrb_, klass, method_name_s, m);
  }

  template <class Func>
  void bind_custom_method(const char* class_name, const char* method_name, Func func_ptr) {
    bind_custom_method(NULL, class_name, method_name, func_ptr);
  }

  //add convertable class pair
  void add_convertable(const char* class_name_first, const char* class_name_second)
  {
    MrubyBindStatus::search(mrb_)->set_class_conversion(class_name_first, class_name_second, true);
    MrubyBindStatus::search(mrb_)->set_class_conversion(class_name_second, class_name_first, true);
  }

  mrb_state* get_mrb(){
      return mrb_;
  }
  mrb_value get_avoid_gc_table(){
      return avoid_gc_table_;
  }

private:
  void Initialize();

  // Returns mruby class under a module.
  std::vector<std::string> SplitModule(const char* module_name);
  struct RClass* DefineModule(const char* module_name);
  struct RClass* DefineClass(const char* module_name, const char* class_name);
  struct RClass* GetClass(const char* class_name);
  struct RClass* GetClass(const char* module_name, const char* class_name);

  // Utility for binding instance method.
  void BindInstanceMethod(const char* class_name, const char* method_name,
                          mrb_value original_func_v,
                          mrb_value (*binder_func)(mrb_state*, mrb_value));
  void BindInstanceMethod(const char* module_name,
                          const char* class_name, const char* method_name,
                          mrb_value original_func_v,
                          mrb_value (*binder_func)(mrb_state*, mrb_value));

  // Mimic mruby API.
  // TODO: Send pull request to the official mruby repository.
  void
  mrb_define_class_method_raw(mrb_state *mrb, struct RClass *c, mrb_sym mid, struct RProc *p);

  static mrb_value mrb_get_untouchable_table( mrb_state* mrb, mrb_value self );
  static mrb_value mrb_get_untouchable_object( mrb_state* mrb, mrb_value self );

  mrb_state* mrb_;
  RClass* mod_;
  mrb_value avoid_gc_table_;
  int arena_index_;
};

MrubyRef load_string(mrb_state* mrb, std::string code);

}  // namespace mrubybind

#endif
