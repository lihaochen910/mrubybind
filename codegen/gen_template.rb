
MAX_PARAM = 6

HEADER = <<EOD
// This file is generated from gen_template.rb
\#define ARG(i)  Type<P##i>::get(args[i])
\#define CHECK(i)  Type<P##i>::check(args[i])

EOD

FUNC_TMPL = <<EOD
// void f(%PARAMS%);
template<%CLASSES0%>
struct Binder<void (*)(%PARAMS%)> {
  static mrb_value call(mrb_state* mrb, void* p, mrb_value* args, int narg) {
    ASSERT(narg == %NPARAM%);%ASSERTS%
    void (*fp)(%PARAMS%) = (void (*)(%PARAMS%))p;
    fp(%ARGS%);
    return mrb_nil_value();
  }
};

// R f(%PARAMS%);
template<class R%CLASSES1%>
struct Binder<R (*)(%PARAMS%)> {
  static mrb_value call(mrb_state* mrb, void* p, mrb_value* args, int narg) {
    ASSERT(narg == %NPARAM%); %ASSERTS%
    R (*fp)(%PARAMS%) = (R (*)(%PARAMS%))p;
    R result = fp(%ARGS%);
    return Type<R>::ret(mrb, result);
  }
};

// C* ctor(%PARAMS%);
template<class C%CLASSES1%>
struct ClassBinder<C* (*)(%PARAMS%)> {
  static void ctor(mrb_state* mrb, mrb_value self, void* p, mrb_value* args, int narg) {
    ASSERT(narg == %NPARAM%); %ASSERTS%
    C* (*ctor)(%PARAMS%) = (C* (*)(%PARAMS%))p;
    C* instance = ctor(%ARGS%);
    DATA_TYPE(self) = &ClassBinder<C>::type_info;
    DATA_PTR(self) = instance;
  }
};

EOD

METHOD_TMPL = <<EOD
// class C { void f(%PARAMS%) };
template<class C%CLASSES1%>
struct ClassBinder<void (C::*)(%PARAMS%)> {
  static mrb_value call(mrb_state* mrb, mrb_value self, void* pp, mrb_value* args, int narg) {
    ASSERT(narg == %NPARAM%);%ASSERTS%
    C* instance = static_cast<C*>(mrb_get_datatype(mrb, self, &ClassBinder<C>::type_info));
    typedef void (C::*M)(%PARAMS%);
    M method = *(M*)pp;
    (instance->*method)(%ARGS%);
    return mrb_nil_value();
  }
};

// class C { R f(%PARAMS%) };
template<class C, class R%CLASSES1%>
struct ClassBinder<R (C::*)(%PARAMS%)> {
  static mrb_value call(mrb_state* mrb, mrb_value self, void* pp, mrb_value* args, int narg) {
    ASSERT(narg == %NPARAM%);%ASSERTS%
    C* instance = static_cast<C*>(mrb_get_datatype(mrb, self, &ClassBinder<C>::type_info));
    typedef R (C::*M)(%PARAMS%);
    M method = *(M*)pp;
    R result = (instance->*method)(%ARGS%);
    return Type<R>::ret(mrb, result);
  }
};

EOD

FOOTER = <<EOD
\#undef ARG
\#undef CHECK
EOD

def embed_template(str, nparam)
  if nparam == 0
    params = 'void'
    args = ''
    classes = ''
    asserts = ''
  else
    params = (0...nparam).map {|i| "P#{i}"}.join(', ')
    args = (0...nparam).map {|i| "ARG(#{i})"}.join(', ')
    classes = (0...nparam).map {|i| "class P#{i}"}.join(', ')
    asserts = (0...nparam).map {|i| "ASSERT(CHECK(#{i}));"}.join(' ')
  end

  table = {
    '%PARAMS%' => params,
    '%NPARAM%' => nparam.to_s,
    '%ARGS%' => args,
    '%CLASSES0%' => classes,
    '%CLASSES1%' => classes.empty? ? '' : ', ' + classes,
    '%ASSERTS%' => asserts
  }

  return str.gsub(/(#{table.keys.join('|')})/) {|k| table[k]}
end

print HEADER
(0..MAX_PARAM).each do |nparam|
  print embed_template(FUNC_TMPL, nparam)
  print embed_template(METHOD_TMPL, nparam)
end
print FOOTER

#
