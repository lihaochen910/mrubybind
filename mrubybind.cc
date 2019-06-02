// Do not modify this file directly, this is generated
#include "mrubybind.h"
#include "mruby/compile.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include <iostream>

namespace mrubybind {

//#include "mrubybind.dat"


const char Type<int>::TYPE_NAME[] = "Fixnum";
const char Type<unsigned int>::TYPE_NAME[] = "Fixnum";
const char Type<float>::TYPE_NAME[] = "Float";
const char Type<double>::TYPE_NAME[] = "Float";
const char Type<const char*>::TYPE_NAME[] = "String";
const char Type<std::string>::TYPE_NAME[] = "String";
const char Type<const std::string>::TYPE_NAME[] = "String";
const char Type<const std::string&>::TYPE_NAME[] = "String";
const char Type<bool>::TYPE_NAME[] = "Bool";
const char Type<void*>::TYPE_NAME[] = "Voidp";
const char Type<MrubyRef>::TYPE_NAME[] = "MrubyRef";
const char Type<MrubyStrongRef>::TYPE_NAME[] = "MrubyStrongRef";
const char Type<MrubyWeakRef>::TYPE_NAME[] = "MrubyWeakRef";
const char TypeFuncBase::TYPE_NAME[] = "Func";
const char TypeClassBase::TYPE_NAME[] = "CppClass";

const char* untouchable_table = "__ untouchable table __";
const char* untouchable_object = "__ untouchable object __";
const char* untouchable_last_exception = "__ untouchable last exception __";

//----------------------------------------------------------------//
mrb_value raise(mrb_state* mrb, int parameter_index,
                const char* required_type_name, mrb_value value) {

  const char* argument_class_name = mrb_obj_classname(mrb, value);
  mrb_raisef(mrb, E_TYPE_ERROR, "can't convert %S into %S, argument %S(%S)",
             mrb_str_new_cstr(mrb, argument_class_name),
             mrb_str_new_cstr(mrb, required_type_name),
             mrb_fixnum_value(parameter_index + 1), value);
  return mrb_nil_value();
}

//----------------------------------------------------------------//
mrb_value raisenarg(mrb_state *mrb, mrb_value func_name, int narg, int nparam) {

  mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S': wrong number of arguments (%S for %S)",
             func_name,
             mrb_fixnum_value(narg),
             mrb_fixnum_value(nparam));
  return mrb_nil_value();
}

//================================================================//
// MrubyBind
//================================================================//

//----------------------------------------------------------------//
mrb_value MrubyBind::mrb_get_untouchable_table ( mrb_state* mrb, mrb_value self ) {

	RClass* mrubybind = mrb_define_module ( mrb, "MrubyBind" );
	return mrb_obj_iv_get( mrb, ( RObject* )mrubybind,
						   mrb_intern_cstr ( mrb, untouchable_table ) );
}

//----------------------------------------------------------------//
mrb_value MrubyBind::mrb_get_untouchable_object ( mrb_state* mrb, mrb_value self ) {

	RClass* mrubybind = mrb_define_module ( mrb, "MrubyBind" );
	return mrb_obj_iv_get( mrb, ( RObject* )mrubybind,
						   mrb_intern_cstr ( mrb, untouchable_object ) );
}

//----------------------------------------------------------------//
void MrubyBind::mrb_define_class_method_raw(mrb_state *mrb, struct RClass* c, mrb_sym mid, struct RProc* p) {

	mrb_define_class_method( mrb, c, mrb_sym2name( mrb, mid ), NULL, MRB_ARGS_ANY() ); // Dummy registration.
	mrb_method_t m;
	MRB_METHOD_FROM_PROC( m, p );
	mrb_define_method_raw( mrb, (( RObject* )c)->c, mid, m );
}

//----------------------------------------------------------------//
MrubyBind::MrubyBind(mrb_state* mrb) : mrb_(mrb), mod_(mrb_->kernel_module) {
	Initialize();
}

//----------------------------------------------------------------//
MrubyBind::MrubyBind(mrb_state* mrb, RClass* mod) : mrb_(mrb), mod_(mod) {
	Initialize();
}

//----------------------------------------------------------------//
MrubyBind::~MrubyBind() {
	mrb_gc_arena_restore( mrb_, arena_index_ );
}

//----------------------------------------------------------------//
void MrubyBind::Initialize() {

	arena_index_ = mrb_gc_arena_save ( mrb_ );
	mrb_sym sym_mrubybind = mrb_intern_cstr ( mrb_, "MrubyBind" );
	RClass* mrubybind = mrb_define_module ( mrb_, "MrubyBind" );

	if ( mrb_const_defined ( mrb_, mrb_obj_value ( mrb_->kernel_module ), sym_mrubybind ) ) {

		avoid_gc_table_ = mrb_obj_iv_get ( mrb_, (RObject* )mrubybind, mrb_intern_cstr ( mrb_, untouchable_table ) );
	}
	else {

		mrb_const_set ( mrb_, mrb_obj_value ( mrb_->kernel_module ), sym_mrubybind, mrb_obj_value ( mrubybind ) );

		{
			avoid_gc_table_ = mrb_ary_new ( mrb_ );

			mrb_obj_iv_set ( mrb_, (RObject* )mrubybind, mrb_intern_cstr ( mrb_, untouchable_table ), avoid_gc_table_ );

			std::shared_ptr<MrubyBindStatus> mrbs = std::make_shared<MrubyBindStatus> ( mrb_, avoid_gc_table_ );

			mrubybind::MrubyBind b ( mrb_ );
			b.bind_class< std::shared_ptr<MrubyBindStatus> > ( "MrubyBind", "MrubyBindStatusPtr" );

			mrb_value msbpv = Type< std::shared_ptr<MrubyBindStatus> >::ret ( mrb_, mrbs );
			mrb_obj_iv_set ( mrb_, (RObject* )mrubybind, mrb_intern_cstr ( mrb_, untouchable_object ), msbpv );

			mrb_define_class_method ( mrb_, mrubybind, "get_untouchable_table", mrb_get_untouchable_table, MRB_ARGS_NONE () );
			mrb_define_class_method ( mrb_, mrubybind, "get_untouchable_object", mrb_get_untouchable_object, MRB_ARGS_NONE () );
		}
	}
}

//----------------------------------------------------------------//
std::vector<std::string> MrubyBind::SplitModule(const char* module_name) {

	std::vector<std::string> splited;
	if ( !module_name ) {
		return splited;
	}
	std::string str( module_name );
	std::string d( "::" );
	size_t c = 0, f, dsz = d.size();
	while ( (f = str.find( d, c )) != std::string::npos ) {
		splited.push_back( str.substr( c, f - c ) );
		c = f + dsz;
	}
	splited.push_back( str.substr( c, str.size() - c ) );
	return splited;
}

//----------------------------------------------------------------//
struct RClass* MrubyBind::DefineModule(const char* module_name) {

	std::vector<std::string> modules = SplitModule( module_name );
	struct RClass* last_module = nullptr;
	for ( size_t i = 0; i < modules.size(); i++ ) {
		auto m = modules[ i ];
		if ( i == 0 ) {
			if ( mrb_const_defined_at( mrb_, mrb_obj_value( mrb_->object_class ), mrb_intern_cstr( mrb_, m.c_str() ) ) ) {
				mrb_value mdl = mrb_const_get( mrb_, mrb_obj_value( mrb_->object_class ), mrb_intern_cstr( mrb_, m.c_str() ) );
				last_module = mrb_class_ptr( mdl );
			}
			else {
				last_module = mrb_define_module( mrb_, m.c_str() );
			}
		}
		else {
			if ( mrb_const_defined_at( mrb_, mrb_obj_value( mrb_->object_class ), mrb_intern_cstr( mrb_, m.c_str() ) ) ) {
				mrb_value mdl = mrb_const_get( mrb_, mrb_obj_value( last_module ), mrb_intern_cstr( mrb_, m.c_str() ) );
				last_module = mrb_class_ptr( mdl );
			}
			else {
				last_module = mrb_define_module_under( mrb_, last_module, m.c_str() );
			}
		}
	}
    return last_module;
}

//----------------------------------------------------------------//
struct RClass* MrubyBind::DefineClass(const char* module_name, const char* class_name) {

	struct RClass* tc;
	mrb_value mod = mrb_obj_value( mod_ );
	std::string name;
	if ( module_name ) {
		name = module_name;
		name += "::";
		name += class_name;
		tc = mrb_define_class( mrb_, name.c_str(), mrb_->object_class );
		struct RClass* mdp = DefineModule( module_name );
		mod = mrb_obj_value( mdp );
		mrb_define_const( mrb_, mdp, class_name, mrb_obj_value( tc ) );
	}
	else {
		name = class_name;
		tc = mrb_define_class( mrb_, class_name, mrb_->object_class );
	}
	return tc;
}

//----------------------------------------------------------------//
struct RClass* MrubyBind::GetClass( const char* class_name ) {

	mrb_value mod = mrb_obj_value( mod_ );
	mrb_value klass_v = mrb_const_get( mrb_, mod, mrb_intern_cstr( mrb_, class_name ) );
	return mrb_class_ptr( klass_v );
}

//----------------------------------------------------------------//
struct RClass* MrubyBind::GetClass(const char* module_name, const char* class_name) {

	mrb_value mod = mrb_obj_value( mod_ );
	if ( module_name ) {
		mod = mrb_obj_value( DefineModule( module_name ) );
	}
	mrb_value klass_v = mrb_const_get( mrb_, mod, mrb_intern_cstr( mrb_, class_name ) );
	return mrb_class_ptr( klass_v );
}

//----------------------------------------------------------------//
void MrubyBind::BindInstanceMethod(
    const char* class_name, const char* method_name,
    mrb_value original_func_v,
    mrb_value (*binder_func)(mrb_state*, mrb_value)) {

  BindInstanceMethod ( NULL,
    class_name, method_name,
    original_func_v,
    binder_func );
}

//----------------------------------------------------------------//
void MrubyBind::BindInstanceMethod(const char* module_name,
                          const char* class_name, const char* method_name,
                          mrb_value original_func_v,
                          mrb_value (*binder_func)(mrb_state*, mrb_value)) {

	mrb_sym method_name_s = mrb_intern_cstr( mrb_, method_name );
	mrb_value env[] = {
	  original_func_v,						// 0: c function pointer
	  mrb_symbol_value( method_name_s ),	// 1: method name
	};
	struct RProc* proc = mrb_proc_new_cfunc_with_env( mrb_, binder_func, 2, env );
	struct RClass* klass = GetClass( module_name, class_name );
	mrb_method_t m;
	MRB_METHOD_FROM_PROC( m, proc );
	mrb_define_method_raw( mrb_, klass, method_name_s, m );
}


//================================================================//
// MrubyRef
//================================================================//

//----------------------------------------------------------------//
MrubyRef load_string ( mrb_state* mrb, std::string code ) {

	mrubybind::MrubyArenaStore mas ( mrb );

	RClass* mrubybind = mrb_define_module ( mrb, "MrubyBind" );
	mrb->exc = NULL;
	mrb_value r = mrb_load_string ( mrb,
									code.c_str () );
	if ( mrb->exc ) {
		mrb_obj_iv_set ( mrb, ( RObject* )mrubybind,
						 mrb_intern_cstr ( mrb, untouchable_last_exception ), mrb_obj_value ( mrb->exc ) );
		r = mrb_nil_value ();
	}
	else {
		mrb_obj_iv_set ( mrb, ( RObject* )mrubybind,
						 mrb_intern_cstr ( mrb, untouchable_last_exception ), mrb_nil_value () );
	}

	return MrubyRef ( mrb, r );
}

//----------------------------------------------------------------//
MrubyRef::MrubyRef() {
}

//----------------------------------------------------------------//
MrubyRef::MrubyRef ( mrb_state* mrb, const mrb_value& v ) {

	this->mrb = mrb;
	mrb_value* p = new mrb_value ();
	*p = v;
	this->v = std::shared_ptr < mrb_value >( p, set_avoid_gc < mrb_value >( mrb, v ) );
}

//----------------------------------------------------------------//
MrubyRef::MrubyRef ( const MrubyRef& assign ) {
	this->take ( assign );
}

//----------------------------------------------------------------//
MrubyRef::~MrubyRef () {
}

//----------------------------------------------------------------//
void MrubyRef::clear () {

	// TODO
	this->v.reset ();
	this->strong_v.reset ();
	this->weak_v.reset ();
}

//----------------------------------------------------------------//
void MrubyRef::take ( const MrubyRef& assign ) {

	this->clear ();

	this->mrb = assign.mrb;
	this->v = assign.v;

	// cast the const away
	( ( MrubyRef& )assign ).v.reset ();
}

//----------------------------------------------------------------//
typedef std::weak_ptr < RObject > weakref;
typedef std::shared_ptr < RObject > strongref;

void strongref_free ( mrb_state* M, void* ptr ) {

	reinterpret_cast< strongref* >( ptr )->~strongref ();
	mrb_free ( M, ptr );
}

mrb_data_type const strongref_type = { "StrongRef", &strongref_free };

//----------------------------------------------------------------//
void MrubyRef::make_strong ( mrb_state* mrb, const mrb_value& v ) {

	mrb_value* p = new mrb_value ();
	*p = v;
	this->v = std::shared_ptr < mrb_value > ( p, set_avoid_gc < mrb_value > ( mrb, v ) );
	/*printf ( "MrubyRef::make_strong() 0x%p\n", mrb_basic_ptr ( v ) );
	mrb_gc_register( mrb, v );
	this->strong_v = std::shared_ptr < RBasic > ( mrb_basic_ptr ( v ), GCDeleter ( mrb ) );*/
}

//----------------------------------------------------------------//
void MrubyRef::make_weak ( mrb_state* mrb, const mrb_value& v ) {

	//printf ( "MrubyRef::make_weak() 0x%p\n", mrb_obj_ptr ( v ) );

	if ( mrb_nil_p ( v ) ) {
		this->weak_v.reset ();
		return;
	}

	strongref* ref = nullptr;

	mrb_sym const sym = mrb_intern_cstr ( mrb, "@__strongref" );

	if ( mrb_iv_defined ( mrb, v, sym ) ) {
		ref = reinterpret_cast< strongref* > ( mrb_data_check_get_ptr ( mrb, mrb_iv_get ( mrb, v, sym ), &strongref_type ) );
	}
	else {
		ref = new ( mrb_malloc ( mrb, sizeof ( strongref ) ) )
			strongref ( mrb_obj_ptr ( v ), NullDeleter < RObject > () );

		mrb_iv_set ( mrb, v, sym, mrb_obj_value (
			mrb_data_object_alloc ( mrb, mrb->object_class, ref, &strongref_type ) ) );
	}

	//weakref* const weak_ref = new ( mrb_malloc ( mrb, sizeof ( weakref ) ) ) weakref ( ref_ref );

	this->weak_v = *ref;

	/*printf ( "use_count %d\n", this->weak_v.use_count() );
	printf ( "weak->shared_ptr use_count %d\n", this->weak_v.lock ().use_count () );*/
}

//----------------------------------------------------------------//
void MrubyRef::set_ref ( mrb_state* mrb, const mrb_value& v ) {

	( void )mrb;
	( void )v;
	//assert ( false );
}

//----------------------------------------------------------------//
void MrubyRef::set_ref ( mrb_state* mrb, const mrb_value& v, unsigned int type ) {

	this->mrb = mrb;

	if ( type == MAKE_STRONG ) {
		this->make_strong ( mrb, v );
	}
	else {
		this->make_weak ( mrb, v );
	}
}

//----------------------------------------------------------------//
bool MrubyRef::is_living () const {
	return MrubyBindStatus::is_living ( mrb );
}

//----------------------------------------------------------------//
mrb_state* MrubyRef::get_mrb () const {
	return this->mrb;
}

//----------------------------------------------------------------//
mrb_value MrubyRef::get_v () const {
	
	if ( v.get () ) {
		return *( this->v.get () );
	}
	return mrb_nil_value ();
}

//----------------------------------------------------------------//
bool MrubyRef::empty () const {
	return !v.get ();
}

//----------------------------------------------------------------//
bool MrubyRef::test () const {
	if ( v.get () ) {
		mrb_value v = *( this->v.get () );
		return mrb_test ( v );
	}
	return false;
}

//----------------------------------------------------------------//
bool MrubyRef::obj_equal ( const MrubyRef& r ) const {
	if ( !empty () && !r.empty () ) {
		return mrb_obj_equal ( mrb, get_v (), r.get_v () );
	}
	else {
		return empty () == r.empty ();
	}
}

//----------------------------------------------------------------//
std::string MrubyRef::to_s () const {

	if ( v.get () ) {
		MrubyArenaStore mas ( mrb );
		mrb_value v = *( this->v.get () );
		if ( mrb_string_p ( v ) ) {
			return std::string ( RSTRING_PTR ( v ), RSTRING_LEN ( v ) );
		}
		else {
			v = mrb_funcall ( mrb, v, "to_s", 0 );
			return std::string ( RSTRING_PTR ( v ), RSTRING_LEN ( v ) );
		}
	}
	else {
		return "";
	}
}

//----------------------------------------------------------------//
int MrubyRef::to_i () const {

	if ( v.get () ) {
		mrb_value v = *( this->v.get () );
		if ( mrb_fixnum_p ( v ) ) {
			return mrb_fixnum ( v );
		}
		else {
			v = mrb_funcall ( mrb, v, "to_i", 0 );
			return mrb_fixnum ( v );
		}
	}
	else {
		return 0;
	}
}

//----------------------------------------------------------------//
float MrubyRef::to_float () const {

	if ( v.get () ) {
		mrb_value v = *( this->v.get () );
		if ( mrb_float_p ( v ) ) {
			return mrb_float ( v );
		}
		else {
			v = mrb_funcall ( mrb, v, "to_f", 0 );
			return mrb_float ( v );
		}
	}
	else {
		return 0.0f;
	}
}

//----------------------------------------------------------------//
double MrubyRef::to_double () const {

	if ( v.get () ) {
		mrb_value v = *( this->v.get () );
		if ( mrb_float_p ( v ) ) {
			return mrb_float ( v );
		}
		else {
			v = mrb_funcall ( mrb, v, "to_f", 0 );
			return ( double )mrb_float ( v );
		}
	}
	else {
		return 0.0;
	}
}

//----------------------------------------------------------------//
MrubyRef MrubyRef::call ( std::string name ) {
	MrubyArenaStore mas ( mrb );
	return MrubyRef ( mrb, mrb_funcall ( mrb, *( this->v.get () ), name.c_str (), 0 ) );
}


//================================================================//
// MrubyStrongRef
//================================================================//

//----------------------------------------------------------------//
MrubyStrongRef::MrubyStrongRef () {
}

//----------------------------------------------------------------//
MrubyStrongRef::MrubyStrongRef ( mrb_state* mrb, const mrb_value& v ) {
	this->set_ref ( mrb, v );
}

//----------------------------------------------------------------//
MrubyStrongRef::MrubyStrongRef ( const MrubyStrongRef& assign ) {
	this->mrb = assign.mrb;
	this->strong_v = assign.strong_v;
}

//----------------------------------------------------------------//
void MrubyStrongRef::set_ref ( mrb_state* mrb, const mrb_value& v ) {

	MrubyRef::set_ref ( mrb, v, MAKE_STRONG );
}

//----------------------------------------------------------------//
mrb_value MrubyStrongRef::get_v () const {

	printf ( "MrubyStrongRef::get_v() 0x%p\n", this->strong_v.get () );

	if ( this->strong_v.get () ) {
		printf ( "MrubyStrongRef::get_v() successed\n" );
		return mrb_obj_value ( this->strong_v.get () );
	}

	printf ( "MrubyStrongRef::get_v() failed\n" );
	return mrb_nil_value ();
}

//----------------------------------------------------------------//
bool MrubyStrongRef::empty () const {
	return !strong_v.get ();
}


//================================================================//
// MrubyWeakRef
//================================================================//

//----------------------------------------------------------------//
MrubyWeakRef::MrubyWeakRef () {
}

//----------------------------------------------------------------//
MrubyWeakRef::MrubyWeakRef ( mrb_state* mrb, const mrb_value& v ) {
	this->set_ref ( mrb, v );
}

//----------------------------------------------------------------//
MrubyWeakRef::MrubyWeakRef ( const MrubyWeakRef& assign ) {
	this->mrb = assign.mrb;
	this->weak_v = assign.weak_v;
}

//----------------------------------------------------------------//
void MrubyWeakRef::set_ref ( mrb_state* mrb, const mrb_value& v ) {

	MrubyRef::set_ref ( mrb, v, MAKE_WEAK );
}

//----------------------------------------------------------------//
mrb_value MrubyWeakRef::get_v () const {
	
	/*printf ( "weak use_count %d\n", this->weak_v.use_count () );
	printf ( "weak->shared_ptr use_count %d\n", this->weak_v.lock ().use_count () );
	printf ( "get_v weak_v 过期? -> %d %p %p\n", this->weak_v.expired (), this->weak_v.lock (), this->weak_v.lock ().get () );*/

	if ( !this->weak_v.expired () ) {

		//printf ( "MrubyWeakRef::get_v() not expired -> 0x%p\n", this->weak_v.lock ().get () );

		//mrb_value weakref_instance = *( this->v.get () );
		//mrb_value obj = mrb_obj_value ( *( this->v.get () ) );

		/*if ( mrb_bool ( mrb_funcall ( mrb, weakref_instance, "weakref_alive?", 0 ) ) ) {

			return mrb_funcall ( mrb, weakref_instance, "__getobj__", 0 );
		}*/

		return mrb_obj_value ( this->weak_v.lock ().get () );
	}

	//printf ( "MrubyWeakRef::get_v() expired -> nil\n" );

	return mrb_nil_value ();
}

//----------------------------------------------------------------//
bool MrubyWeakRef::empty () const {

	//printf ( "empty weak_v 过期? -> %d %p\n", this->weak_v.expired (), this->weak_v.lock().get() );

	return this->weak_v.expired ();
}


}  // namespace mrubybind
