mrubybind - Binding library for mruby/C++
=========================================

mrubybind automatically creates C function/class-method binder for [mruby](https://github.com/mruby/mruby),
using C++ template partial specialization.
require C++11. because it use std::function and lambda.

## Usage

### How to use mrubybind in your project

1. Put following source codes into your project.
  * mrubybind.cc
  * mrubybind.h
2. Include "mrubybind.h"
3. Use `MrubyBind` instance to bind C function/class-method to mruby.

## Examples

### Bind C function and call it from mruby

1. C function (Any type you want):

  ```c++
  int square(int x) {
    return x * x;
  }
  ```

2. Bind it using mrubybind `bind` method:

  ```c++
  #include "mrubybind.h"

  void install_square_function(mrb_state* mrb) {
    mrubybind::MrubyBind b(mrb);
    b.bind("square", square);
  }
  ```

  You can throw away `MrubyBind` instance after binding function.

3. Call it from mruby:

  ```ruby
  puts square(1111)  #=> 1234321
  ```

### Bind C++ class and method and create its instance from mruby

1. C++ class:

  ```c++
  class Foo {
  public:
    Foo(int x) : x_(x) {
      cout << "Foo::ctor(" << x << ")" << endl;
    }
    virtual ~Foo() {
      cout << "Foo::dtor()" << endl;
    }
    int bar(int y) {
      return x_ + y;
    }
    static int baz(int z) {
      return z * z;
    }

  private:
    int x_;
  };
  ```

2. Bind C++ class using mrubybind `bind_class`, `bind_instance_method` and
   `bind_static_method` method:

  ```c++
  #include "mrubybind.h"

  // Helper function for constructor.
  Foo* new_foo(int x) {
    return new Foo(x);
  }

  void install_foo_class(mrb_state* mrb) {
    mrubybind::MrubyBind b(mrb);
    b.bind_class("Foo", new_foo);
    b.bind_instance_method("Foo", "bar", &Foo::bar);
    b.bind_static_method("Foo", "baz", &Foo::baz);
  }
  ```

3. Call it from mruby:

  ```ruby
  foo = Foo.new(123)  #=> Foo::ctor(123)
  p foo               #=> #<Foo:0x7fa828803d80>
  p foo.bar(567)      #=> 690
  p Foo.baz(9999)     #=> 99980001
                      #=> Foo::dtor()
  ```

### Bind functions under some module

1. Pass `RClass*` instace for `MrubyBind` constructor:

  ```c++
  void install(mrb_state* mrb) {
    RClass* mod = mrb_define_module(mrb, "YourModule");
    mrubybind::MrubyBind b(mrb, mod);
    b.bind("foo", foo);
  }
  ```

  You can use `YourModule.foo` function from mruby.

### Bind constant

1. Use `bind_const` method:

  ```c++
  void install(mrb_state* mrb) {
    mrubybind::MrubyBind b(mrb);
    b.bind_const("FOO", FOO_VALUE);
  }
  ```

### Call block from C++ code

1. define C++ function having callback function

  ```c++
  std::string call_block(mrubybind::FuncPtr<void(int a0)> f) {
    if(f)
    {
        f.func()(23);
    }
    return "called\n";
  }
  ```

2. Bind it using mrubybind:

  ```c++
  #include "mrubybind.h"
  
  void install_call_block_function(mrb_state* mrb) {
    mrubybind::MrubyBind b(mrb);
    b.bind("call_block", call_block);
  }
  ```

3. Call it from mruby:

  ```ruby
  puts call_block { |a0|
    puts "a0 = #{a0}"
  }
  ```

### Manage Registered Class Instance

1. Define C++ function managing class instance:

  ```c++
  class ClassValue{
  public:
    int a;
    
    ClassValue(){
        a = 7;
    }
    
    ~ClassValue(){
        
    }
    
    void decriment(){
        a--;
    }
  };

  std::shared_ptr<ClassValue> create_class_value()
  {
    return std::shared_ptr<ClassValue>(new ClassValue());
  }

  void class_value_increment(std::shared_ptr<ClassValue> cv)
  {
    cv->a++;
  }

  int class_value_get_a(std::shared_ptr<ClassValue> cv)
  {
    return cv->a;
  }
  
  void class_value_decriment(std::shared_ptr<ClassValue> cv)
  {
    cv->decriment();
  }
  ```
  
2. Register class and bind function:

  ```c++
  #include "mrubybind.h"
  
  void install_class_value_function(mrb_state* mrb) {
    mrubybind::MrubyBind b(mrb);
    b.bind("create_class_value", create_class_value);
    b.bind_class<std::shared_ptr<ClassValue> >("ClassValue");
    b.bind("class_value_increment", class_value_increment);
    b.bind("class_value_get_a", class_value_get_a);
    b.bind_custom_method(NULL, "ClassValue", "decriment", class_value_decriment);
  }
  ```  
  
3. Call it from mruby:

   ```ruby
   cv = create_class_value
   puts "cv -> #{class_value_get_a(cv)}"
   class_value_increment(cv)
   puts "cv -> #{class_value_get_a cv}"
   cv.decriment
   puts "cv -> #{class_value_get_a cv}"
   ```

### Refering to mruby object

1. Recieve mruby object reference function:

  ```c++
  mrubybind::MrubyStrongRef mruby_ref;
  mrubybind::MrubyWeakRef mruby_weak_ref;
  
  void set_mruby_ref(mrubybind::MrubyStrongRef r){
    mruby_ref = r;
  }

  void set_mruby_weak_ref(mrubybind::MrubyWeakRef r){
    mruby_weak_ref = r;
  }
  ```
  
2. Bind it using mrubybind:

  ```c++
  #include "mrubybind.h"
  
  void install_mruby_ref_function(mrb_state* mrb) {
    mrubybind::MrubyBind b(mrb);
    b.bind("set_mruby_ref", set_mruby_ref);
    b.bind("set_mruby_weak_ref", set_mruby_weak_ref);
  }
  ```
  
3. Send from mruby code:

  ```ruby
  set_mruby_ref "3test"
  set_mruby_weak_ref (Object.new)

  GC.start
  ```

4. Manage reference of mruby object on C++:  
  
  ```c++
  std::cout << "mruby_ref = " << mruby_ref.to_s() << std::endl;
  std::cout << "mruby_ref = " << mruby_ref.to_i() << std::endl;
  std::cout << "mruby_ref = " << mruby_ref.call("gsub", "te", "toa").to_s() << std::endl;
  std::cout << "mruby_weak_ref released? " << mruby_ref.empty() << std::endl;
  ```

## Supported types
| C++ type                 | mruby type              |
|--------------------------|-------------------------|
| int, unsigned int        | Fixnum                  |
| float, double            | Float                   |
| const char*, string      | String                  |
| bool                     | TrueClass or FalseClass |
| void*                    | Object                  |
| mrubybind::FuncPtr<...>  | Proc                    |
| mrubybind::MrubyRef      | Any Mruby Object        |
| mrubybind::MrubyWeakRef  | Weak Reference Any Mruby Object |
| mrubybind::MrubyStrongRef| Strong Reference Any Mruby Object |
| registered class         | registered class        |

See [mrubybind.h](https://github.com/lihaochen910/mrubybind/blob/devel/mrubybind.h).
  
# License

MIT license.
