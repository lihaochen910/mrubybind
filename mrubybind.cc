// Do not modify this file directly, this is generated
#include "mrubybind.h"
#include "mruby/compile.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mruby/string.h"
#include "mruby/variable.h"

namespace mrubybind {

//#include "mrubybind.dat"
#include <stdint.h>
const uint8_t binder[] = {
0x52,0x49,0x54,0x45,0x30,0x30,0x30,0x32,0xb9,0x95,0x00,0x00,0x04,0x33,0x4d,0x41,
0x54,0x5a,0x30,0x30,0x30,0x30,0x49,0x52,0x45,0x50,0x00,0x00,0x04,0x15,0x30,0x30,
0x30,0x30,0x00,0x00,0x00,0x32,0x00,0x01,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x04,
0x00,0x80,0x00,0x05,0x00,0x80,0x00,0x44,0x00,0x80,0x00,0x45,0x00,0x00,0x00,0x4a,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x09,0x4d,0x72,0x75,0x62,0x79,0x42,
0x69,0x6e,0x64,0x00,0x00,0x00,0x00,0x6c,0x00,0x01,0x00,0x02,0x00,0x02,0x00,0x00,
0x00,0x0a,0x00,0x80,0x00,0x91,0x00,0x80,0x40,0x47,0x01,0x00,0x00,0xc0,0x00,0x80,
0x00,0x46,0x00,0x80,0x00,0x91,0x00,0x80,0x40,0x47,0x01,0x00,0x02,0xc0,0x00,0x80,
0x80,0x46,0x00,0x80,0x01,0x04,0x00,0x80,0x00,0x29,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x03,0x00,0x0a,0x62,0x69,0x6e,0x64,0x5f,0x63,0x6c,0x61,0x73,0x73,0x00,0x00,
0x09,0x4d,0x72,0x75,0x62,0x79,0x42,0x69,0x6e,0x64,0x00,0x00,0x12,0x62,0x69,0x6e,
0x64,0x5f,0x73,0x74,0x61,0x74,0x69,0x63,0x5f,0x6d,0x65,0x74,0x68,0x6f,0x64,0x00,
0x00,0x00,0x00,0x4b,0x00,0x07,0x00,0x09,0x00,0x01,0x00,0x00,0x00,0x07,0x0a,0x00,
0x00,0x26,0x03,0x80,0x40,0x01,0x04,0x00,0xc0,0x01,0x03,0x80,0x00,0xa0,0x04,0x00,
0x01,0x40,0x03,0x80,0x40,0x21,0x03,0x80,0x00,0x29,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x02,0x00,0x09,0x63,0x6f,0x6e,0x73,0x74,0x5f,0x67,0x65,0x74,0x00,0x00,0x0a,
0x63,0x6c,0x61,0x73,0x73,0x5f,0x65,0x76,0x61,0x6c,0x00,0x00,0x00,0x00,0x47,0x00,
0x01,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x05,0x00,0x80,0x00,0x06,0x01,0x00,0x00,
0x84,0x01,0x80,0x01,0x40,0x00,0x80,0x00,0xa1,0x00,0x80,0x00,0x29,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x02,0x00,0x0d,0x64,0x65,0x66,0x69,0x6e,0x65,0x5f,0x6d,0x65,
0x74,0x68,0x6f,0x64,0x00,0x00,0x0a,0x69,0x6e,0x69,0x74,0x69,0x61,0x6c,0x69,0x7a,
0x65,0x00,0x00,0x00,0x01,0x08,0x00,0x03,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x1d,
0x00,0x08,0x00,0x26,0x01,0x80,0x40,0x01,0x01,0x80,0x00,0x20,0x02,0x01,0x40,0x95,
0x01,0x80,0x40,0xa0,0x01,0xc0,0x07,0x19,0x01,0x80,0x00,0x06,0x02,0x00,0x01,0x91,
0x02,0x80,0x00,0x3d,0x03,0x00,0x40,0x01,0x03,0x00,0x00,0x20,0x02,0x81,0x80,0x3e,
0x03,0x00,0x00,0xbd,0x02,0x81,0x80,0x3e,0x03,0x01,0x40,0x95,0x02,0x81,0x80,0x3e,
0x03,0x00,0x01,0x3d,0x02,0x81,0x80,0x3e,0x02,0x01,0x00,0xa0,0x01,0x80,0x80,0xa0,
0x01,0x80,0x02,0x91,0x02,0x00,0x80,0x95,0x02,0x80,0x00,0x06,0x03,0x01,0x00,0x95,
0x02,0x01,0x01,0xb7,0x02,0x80,0x40,0x01,0x02,0x01,0x40,0x38,0x01,0x81,0xbf,0xa0,
0x01,0x80,0x00,0x29,0x00,0x00,0x00,0x03,0x00,0x00,0x29,0x27,0x69,0x6e,0x69,0x74,
0x69,0x61,0x6c,0x69,0x7a,0x65,0x27,0x3a,0x20,0x77,0x72,0x6f,0x6e,0x67,0x20,0x6e,
0x75,0x6d,0x62,0x65,0x72,0x20,0x6f,0x66,0x20,0x61,0x72,0x67,0x75,0x6d,0x65,0x6e,
0x74,0x73,0x20,0x28,0x00,0x00,0x05,0x20,0x66,0x6f,0x72,0x20,0x00,0x00,0x01,0x29,
0x00,0x00,0x00,0x07,0x00,0x04,0x73,0x69,0x7a,0x65,0x00,0x00,0x02,0x21,0x3d,0x00,
0x00,0x05,0x72,0x61,0x69,0x73,0x65,0x00,0x00,0x0d,0x41,0x72,0x67,0x75,0x6d,0x65,
0x6e,0x74,0x45,0x72,0x72,0x6f,0x72,0x00,0x00,0x03,0x6e,0x65,0x77,0x00,0x00,0x09,
0x4d,0x72,0x75,0x62,0x79,0x42,0x69,0x6e,0x64,0x00,0x00,0x0d,0x63,0x61,0x6c,0x6c,
0x5f,0x63,0x74,0x6f,0x72,0x66,0x75,0x6e,0x63,0x00,0x00,0x00,0x00,0x4b,0x00,0x08,
0x00,0x0a,0x00,0x01,0x00,0x00,0x00,0x07,0x0c,0x00,0x00,0x26,0x04,0x00,0x40,0x01,
0x04,0x80,0xc0,0x01,0x04,0x00,0x00,0xa0,0x04,0x80,0x01,0x40,0x04,0x00,0x40,0x21,
0x04,0x00,0x00,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x09,0x63,0x6f,
0x6e,0x73,0x74,0x5f,0x67,0x65,0x74,0x00,0x00,0x0a,0x63,0x6c,0x61,0x73,0x73,0x5f,
0x65,0x76,0x61,0x6c,0x00,0x00,0x00,0x00,0x42,0x00,0x01,0x00,0x03,0x00,0x01,0x00,
0x00,0x00,0x05,0x00,0x80,0x00,0x06,0x00,0x80,0x00,0x20,0x01,0x00,0x01,0x40,0x00,
0x80,0x40,0x21,0x00,0x80,0x00,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
0x05,0x63,0x6c,0x61,0x73,0x73,0x00,0x00,0x0d,0x69,0x6e,0x73,0x74,0x61,0x6e,0x63,
0x65,0x5f,0x65,0x76,0x61,0x6c,0x00,0x00,0x00,0x00,0x3a,0x00,0x01,0x00,0x04,0x00,
0x01,0x00,0x00,0x00,0x05,0x00,0x80,0x00,0x06,0x01,0x01,0x00,0x95,0x01,0x80,0x01,
0x40,0x00,0x80,0x00,0xa1,0x00,0x80,0x00,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0x00,0x0d,0x64,0x65,0x66,0x69,0x6e,0x65,0x5f,0x6d,0x65,0x74,0x68,0x6f,0x64,
0x00,0x00,0x00,0x01,0x0a,0x00,0x03,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x20,0x00,
0x08,0x00,0x26,0x01,0x80,0x40,0x01,0x01,0x80,0x00,0x20,0x02,0x01,0x81,0x15,0x01,
0x80,0x40,0xa0,0x01,0xc0,0x09,0x19,0x01,0x80,0x00,0x06,0x02,0x00,0x01,0x91,0x02,
0x80,0x00,0x3d,0x03,0x01,0x01,0x15,0x02,0x81,0x80,0x3e,0x03,0x00,0x00,0xbd,0x02,
0x81,0x80,0x3e,0x03,0x00,0x40,0x01,0x03,0x00,0x00,0x20,0x02,0x81,0x80,0x3e,0x03,
0x00,0x01,0x3d,0x02,0x81,0x80,0x3e,0x03,0x01,0x81,0x15,0x02,0x81,0x80,0x3e,0x03,
0x00,0x01,0xbd,0x02,0x81,0x80,0x3e,0x02,0x01,0x00,0xa0,0x01,0x80,0x80,0xa0,0x01,
0x80,0x02,0x91,0x02,0x00,0x81,0x15,0x02,0x81,0x41,0x15,0x02,0x01,0x01,0x37,0x02,
0x80,0x40,0x01,0x02,0x01,0x40,0x38,0x01,0x81,0xbf,0xa0,0x01,0x80,0x00,0x29,0x00,
0x00,0x00,0x04,0x00,0x00,0x01,0x27,0x00,0x00,0x1e,0x27,0x3a,0x20,0x77,0x72,0x6f,
0x6e,0x67,0x20,0x6e,0x75,0x6d,0x62,0x65,0x72,0x20,0x6f,0x66,0x20,0x61,0x72,0x67,
0x75,0x6d,0x65,0x6e,0x74,0x73,0x20,0x28,0x00,0x00,0x05,0x20,0x66,0x6f,0x72,0x20,
0x00,0x00,0x01,0x29,0x00,0x00,0x00,0x07,0x00,0x04,0x73,0x69,0x7a,0x65,0x00,0x00,
0x02,0x21,0x3d,0x00,0x00,0x05,0x72,0x61,0x69,0x73,0x65,0x00,0x00,0x0d,0x41,0x72,
0x67,0x75,0x6d,0x65,0x6e,0x74,0x45,0x72,0x72,0x6f,0x72,0x00,0x00,0x03,0x6e,0x65,
0x77,0x00,0x00,0x09,0x4d,0x72,0x75,0x62,0x79,0x42,0x69,0x6e,0x64,0x00,0x00,0x0a,
0x63,0x61,0x6c,0x6c,0x5f,0x63,0x66,0x75,0x6e,0x63,0x00,0x45,0x4e,0x44,0x00,0x00,
0x00,0x00,0x08,
};

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

mrb_value raise(mrb_state *mrb, int parameter_index,
                const char* required_type_name, mrb_value value) {
  const char * argument_class_name = mrb_obj_classname(mrb, value);
  mrb_raisef(mrb, E_TYPE_ERROR, "can't convert %S into %S, argument %S(%S)",
             mrb_str_new_cstr(mrb, argument_class_name),
             mrb_str_new_cstr(mrb, required_type_name),
             mrb_fixnum_value(parameter_index + 1), value);
  return mrb_nil_value();
}

mrb_value raise2(mrb_state *mrb, mrb_value func_name, int narg, int nparam) {
  mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S': wrong number of arguments (%S for %S)",
             func_name,
             mrb_fixnum_value(narg),
             mrb_fixnum_value(nparam));
  return mrb_nil_value();
}

static mrb_value call_cfunc(mrb_state *mrb, mrb_value /*self*/) {
  mrb_value binder;
  mrb_value func_ptr_v;
  mrb_value* args;
  int narg;
  mrb_get_args(mrb, "oo*", &binder, &func_ptr_v, &args, &narg);
  typedef mrb_value (*BindFunc)(mrb_state*, void*, mrb_value*, int);
  BindFunc binderp = reinterpret_cast<BindFunc>(mrb_voidp(binder));
  return binderp(mrb, mrb_voidp(func_ptr_v), args, narg);
}

static mrb_value call_ctorfunc(mrb_state *mrb, mrb_value self) {
  mrb_value binder;
  mrb_value self_v;
  mrb_value new_func_ptr_v;
  mrb_value* args;
  int narg;
  mrb_get_args(mrb, "ooo*", &binder, &self_v, &new_func_ptr_v, &args, &narg);
  typedef mrb_value (*BindFunc)(mrb_state*, mrb_value, void*, mrb_value*, int);
  BindFunc binderp = reinterpret_cast<BindFunc>(mrb_voidp(binder));
  binderp(mrb, self_v, mrb_voidp(new_func_ptr_v), args, narg);
  return self;
}

static mrb_value call_imethod(mrb_state *mrb, mrb_value /*self*/) {
  mrb_value binder;
  mrb_value self_v;
  mrb_value method_pptr_v;
  mrb_value* args;
  int narg;
  mrb_get_args(mrb, "ooS*", &binder, &self_v, &method_pptr_v, &args, &narg);
  typedef mrb_value (*BindFunc)(mrb_state*, mrb_value, void*, mrb_value*, int);
  BindFunc binderp = reinterpret_cast<BindFunc>(mrb_voidp(binder));
  return binderp(mrb, self_v, RSTRING_PTR(method_pptr_v), args, narg);
}

static mrb_value call_smethod(mrb_state *mrb, mrb_value self) {
  mrb_value binder;
  mrb_value method_pptr_v;
  mrb_value* args;
  int narg;
  mrb_get_args(mrb, "oS*", &binder, &method_pptr_v, &args, &narg);
  typedef mrb_value (*BindFunc)(mrb_state*, mrb_value, void*, mrb_value*, int);
  BindFunc binderp = reinterpret_cast<BindFunc>(mrb_voidp(binder));
  return binderp(mrb, self, RSTRING_PTR(method_pptr_v), args, narg);
}

MrubyBind::MrubyBind(mrb_state* mrb) : mrb_(mrb), mod_(mrb_->kernel_module) {
  Initialize();
}

MrubyBind::MrubyBind(mrb_state* mrb, RClass* mod) : mrb_(mrb), mod_(mod) {
  Initialize();
}

MrubyBind::~MrubyBind() {
  mrb_gc_arena_restore(mrb_, arena_index_);
}

void MrubyBind::Initialize() {
  arena_index_ = mrb_gc_arena_save(mrb_);
  mrb_sym sym_mrubybind = mrb_intern_cstr(mrb_, "MrubyBind");
  if (mrb_const_defined(mrb_, mrb_obj_value(mrb_->kernel_module),
                        sym_mrubybind)) {
    mod_mrubybind_ = mrb_const_get(mrb_, mrb_obj_value(mrb_->kernel_module),
                                   sym_mrubybind);
  } else {
    RClass* mrubybind = mrb_define_module(mrb_, "MrubyBind");
    mod_mrubybind_ = mrb_obj_value(mrubybind);
    mrb_define_module_function(mrb_, mrubybind, "call_cfunc", call_cfunc,
                               ARGS_REQ(2) | ARGS_REST());
    mrb_define_module_function(mrb_, mrubybind, "call_ctorfunc", call_ctorfunc,
                               ARGS_REQ(3) | ARGS_REST());
    mrb_define_module_function(mrb_, mrubybind, "call_imethod", call_imethod,
                               ARGS_REQ(3) | ARGS_REST());
    mrb_define_module_function(mrb_, mrubybind, "call_smethod", call_smethod,
                               ARGS_REQ(2) | ARGS_REST());
    mrb_irep* irep = mrb_read_irep(mrb_, binder);
    if (irep != NULL) {
      mrb_run(mrb_, mrb_proc_new(mrb_, irep), mrb_top_self(mrb_));
    }
  }
}

}  // namespace mrubybind
