// RUN: rm -rf %t
// RUN: %clang_cc1 -fmodules -x objective-c++ -fmodule-cache-path %t -fmodule-name=module_private_left -emit-module %S/Inputs/module.map
// RUN: %clang_cc1 -fmodules -x objective-c++ -fmodule-cache-path %t -fmodule-name=module_private_right -emit-module %S/Inputs/module.map
// RUN: %clang_cc1 -fmodules -x objective-c++ -fmodule-cache-path %t %s -verify
// FIXME: When we have a syntax for modules in C++, use that.

@__experimental_modules_import module_private_left;
@__experimental_modules_import module_private_right;

void test() {
  int &ir = f0(1.0); // okay: f0() from 'right' is not visible
}

int test_broken() {
  HiddenStruct hidden; // expected-error{{use of undeclared identifier 'HiddenStruct'}}

  Integer i; // expected-error{{use of undeclared identifier 'Integer'}}

  int *ip = 0;
  f1(ip); // expected-error{{use of undeclared identifier 'f1'}}

  vector<int> vec; // expected-error{{use of undeclared identifier 'vector'}} \
  // expected-error{{expected '(' for function-style cast or type construction}} \
  // expected-error{{use of undeclared identifier 'vec'}}

  VisibleStruct vs;
  vs.field = 0; // expected-error{{no member named 'field' in 'VisibleStruct'}}
  vs.setField(1); // expected-error{{no member named 'setField' in 'VisibleStruct'}}

  return hidden_var; // expected-error{{use of undeclared identifier 'hidden_var'}}
}

// Check for private redeclarations of public entities.
template<typename T>
class public_class_template;

template<typename T>
__module_private__ class public_class_template;


typedef int public_typedef;
typedef __module_private__ int public_typedef;

extern int public_var;
extern __module_private__ int public_var;

void public_func();
__module_private__ void public_func();

template<typename T>
void public_func_template();
template<typename T>
__module_private__ void public_func_template();

struct public_struct;
__module_private__ struct public_struct;

// Check for attempts to make specializations private
template<> __module_private__ void public_func_template<int>(); // expected-error{{template specialization cannot be declared __module_private__}}

template<typename T>
struct public_class {
  struct inner_struct;
  static int static_var;

  friend __module_private__ void public_func_friend();
  friend __module_private__ struct public_struct_friend;
};

template<> __module_private__ struct public_class<int>::inner_struct { }; // expected-error{{member specialization cannot be declared __module_private__}}
template<> __module_private__ int public_class<int>::static_var = 17; // expected-error{{member specialization cannot be declared __module_private__}}

template<>
__module_private__ struct public_class<float> { }; // expected-error{{template specialization cannot be declared __module_private__}}

template<typename T>
__module_private__ struct public_class<T *> { }; // expected-error{{partial specialization cannot be declared __module_private__}}

// Check for attempts to make parameters and variables with automatic
// storage module-private.

void local_var_private(__module_private__ int param) { // expected-error{{parameter 'param' cannot be declared __module_private__}}
  __module_private__ struct Local { int x, y; } local; //expected-error{{local variable 'local' cannot be declared __module_private__}}

  __module_private__ struct OtherLocal { int x; }; // expected-error{{local struct cannot be declared __module_private__}}

  typedef __module_private__ int local_typedef; // expected-error{{typedef 'local_typedef' cannot be declared __module_private__}}
}

// Check struct size
struct LikeVisibleStruct {
  int field;
  virtual void setField(int f);
};

int check_struct_size[sizeof(VisibleStruct) == sizeof(LikeVisibleStruct)? 1 : -1];