// Copyright (C) 2008  Maksim Sipos <msipos@mailc.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef VM_H
#define VM_H

#include "clib/clib.h"

//////////////////////////////////////////////////////////////////////////////
// Value
//////////////////////////////////////////////////////////////////////////////
typedef uint64 Value;
#include "vm/value_inline.c"

//////////////////////////////////////////////////////////////////////////////
// sym-table.c
//////////////////////////////////////////////////////////////////////////////
typedef uint32 Dsym;
void sym_init();
Value ssym_get(const char* name);
Value ssym_set(const char* name, Value val);
Dsym dsym_get(const char* name);
const char* dsym_reverse_get(Dsym dsym);

extern Dsym dsym_plus,  dsym_minus,  dsym_star,  dsym_slash;
extern Dsym dsym_plus2, dsym_minus2, dsym_star2, dsym_slash2;
extern Dsym dsym_gt, dsym_gt2;
extern Dsym dsym_lt, dsym_lt2;
extern Dsym dsym_gte, dsym_gte2;
extern Dsym dsym_lte, dsym_lte2;

//////////////////////////////////////////////////////////////////////////////
// exceptions.c
//////////////////////////////////////////////////////////////////////////////

#include <setjmp.h>

void exc_init();
#define exc_register_any() ({ int tmp_exc_var; \
                              tmp_exc_var = setjmp(exc_jb); \
                              if (tmp_exc_var == 0) exc_register_any2(); \
                              tmp_exc_var; })
void exc_register_any2();
jmp_buf exc_jb; // TODO: Make this thread safe.
void exc_raise(char* format, ...) __attribute__ ((noreturn)) ;

//////////////////////////////////////////////////////////////////////////////
// klass.c
//////////////////////////////////////////////////////////////////////////////

typedef enum {
  KLASS_DIRECT,
  KLASS_OBJECT,
  KLASS_CDATA_OBJECT,
  KLASS_FIELD_OBJECT
} KlassType;

typedef struct {
  Dsym name;
  KlassType type;
  Dict methods;
  int num_fields;
  Dict readable_fields;
  Dict writable_fields;
  Dict fields;
  int obj_size;
} Klass;

// Some klasses that have to be public
extern Klass* klass_flag;
extern Klass* klass_integer;
extern Klass* klass_double;
extern Klass* klass_string;
typedef struct {
  Klass* klass;
  Value values[0];
} Object;

Klass* klass_new(Dsym name, Dsym parent, KlassType type, int cdata_size);
void klass_new_method(Klass* klass, Dsym name, Value method);

#define FIELD_READABLE 1
#define FIELD_WRITABLE 2
int klass_new_field(Klass* klass, Dsym name, int type);
int klass_get_field_int(Klass* klass, Dsym name);
void klass_new_virtual_reader(Klass* klass, Dsym name, Value func);
void klass_new_virtual_writer(Klass* klass, Dsym name, Value func);
Klass* klass_get(Dsym name);
static inline const char* klass_name(Klass* klass){
  return dsym_reverse_get(klass->name);
}
void klass_init();
void klass_init_phase15();
void klass_dump();

// Used to initialize KLASS_OBJECT object.
Value obj_new(Klass* klass, void** data);
// Verify an object is of given type.
static inline void obj_verify(Value v_obj, Klass* klass){
  Klass* klass_obj;
  switch(v_obj & MASK_TAIL){
    case 0b00:
      klass_obj = *((Klass**) unpack_ptr(v_obj));
      break;
    case 0b01:
      klass_obj = klass_integer;
      return;
    case 0b10:
      klass_obj = klass_double;
      return;
    case 0b11:
      klass_obj = klass_flag;
      return;
    default:
      assert_never();
  }
  if (klass_obj != klass){
    exc_raise("TypeError: expected a %s, got a %s",
              dsym_reverse_get(klass->name),
              dsym_reverse_get(klass_obj->name));
  }
}
static inline void* obj_c_data(Value v_obj)
{
  return &(((Object*) unpack_ptr(v_obj))->values[0]);
}
static inline Klass* obj_klass(Value v_obj)
{
  switch(v_obj & MASK_TAIL){
    case 0b00:
      return *((Klass**) unpack_ptr(v_obj));
    case 0b01:
      return klass_integer;
    case 0b10:
      return klass_double;
    case 0b11:
      return klass_flag;
  }
  assert_never();
}

Value field_get(Value v_obj, Dsym field);
void field_set(Value v_obj, Dsym field, Value val);

void method_error(Klass* klass, Dsym dsym);
static inline Value method_get(Value v_obj, Dsym dsym)
{
  Klass* klass = obj_klass(v_obj);
  Value method;
  if (dict_query(&(klass->methods), &dsym, &method)){
    return method;
  } else {
    method_error(klass, dsym);
    return VALUE_NIL;
  }
}

//////////////////////////////////////////////////////////////////////////////
// common.c
//////////////////////////////////////////////////////////////////////////////
extern int sys_argc;
extern char** sys_argv;

extern Klass* klass_Array1;
extern Klass* klass_String;

extern Dsym dsym_to_string;

void common_init_phase15();

//////////////////////////////////////////////////////////////////////////////
// builtin/Function.c
//////////////////////////////////////////////////////////////////////////////
extern Klass* klass_func;
void init1_Function();
void init2_Function();
#include "vm/func-generated.h"
void func_set_vararg(Value v_func);

//////////////////////////////////////////////////////////////////////////////
// util.c
//////////////////////////////////////////////////////////////////////////////
const char* to_string(Value v);
uint64 hash(Value v);

//////////////////////////////////////////////////////////////////////////////
// ops.c
//////////////////////////////////////////////////////////////////////////////
Value op_equal(Value a, Value b);
bool op_equal2(Value a, Value b);
Value op_not_equal(Value a, Value b);
Value op_unary_not(Value v);
Value op_unary_minus(Value v);
Value op_and(Value a, Value b);
Value op_or(Value a, Value b);
#include "vm/ops-generated.h"

//////////////////////////////////////////////////////////////////////////////
// Arrays.c
//////////////////////////////////////////////////////////////////////////////

typedef struct {
  uint64 alloc_size;
  uint64 size;
  Value* data;
} Array1;
Array1* val_to_array1(Value array1);
Value array1_to_val(int64 num_elements, Value* data);
Value array1_to_val2(uint16 num_args, ...);
void array1_index_set(Array1* array1, int64 idx, Value val);
Value array1_index(Array1* array1, int64 idx);

extern Klass* klass_Array2;
typedef struct {
  uint64 size_x;
  uint64 size_y;
  Value* data;
} Array2;
extern Klass* klass_Array3;
typedef struct {
  uint64 size_x;
  uint64 size_y;
  uint64 size_z;
  Value* data;
} Array3;
Value array2_index(Value v_array, int64 x, int64 y);
void array2_index_set(Value v_array, int64 x, int64 y, Value v_val);
void init1_Arrays();
void init2_Arrays();

//////////////////////////////////////////////////////////////////////////////
// Double.c
//////////////////////////////////////////////////////////////////////////////
void init1_Double();
void init2_Double();
#define double_to_val(x) pack_double(x)
double val_to_double(Value v);
double val_to_double_soft(Value v);

//////////////////////////////////////////////////////////////////////////////
// Flags.c
//////////////////////////////////////////////////////////////////////////////
void init1_Flags();
void init2_Flags();

//////////////////////////////////////////////////////////////////////////////
// Integer.c
//////////////////////////////////////////////////////////////////////////////
void init1_Integer();
void init2_Integer();
#define int64_to_val(a) pack_int64(a)
int64 val_to_int64(Value v);
int64 val_to_int64_soft(Value v);

//////////////////////////////////////////////////////////////////////////////
// Map.c
//////////////////////////////////////////////////////////////////////////////
typedef enum {
  BUCKET_EMPTY = 0,
  BUCKET_WAS_FULL,
  BUCKET_FULL
} BucketType;

typedef struct {
  BucketType type;
  Value key;
  Value value;
} Bucket;

typedef struct {
  Bucket* buckets;
  int64 size;
  int64 alloc_size;
} Map;

void init1_Map();
void init2_Map();
bool map_query(Map* map, Value key, Value* value);
void map_set(Map* map, Value key, Value value);
void map_init(Map* map);

//////////////////////////////////////////////////////////////////////////////
// Object.c
//////////////////////////////////////////////////////////////////////////////
void init1_Object();
void init2_Object();

//////////////////////////////////////////////////////////////////////////////
// Range.c
//////////////////////////////////////////////////////////////////////////////
extern Klass* klass_Range;
void init1_Range();
void init2_Range();
Value range_to_val(int64 start, int64 finish);
void val_to_range(Value range, int64* start, int64* finish);

//////////////////////////////////////////////////////////////////////////////
// String.c
//////////////////////////////////////////////////////////////////////////////
typedef struct {
  char* str;
} String;

char* val_to_string(Value v);
Value string_to_val(char* str);

//////////////////////////////////////////////////////////////////////////////
// Complex.c
//////////////////////////////////////////////////////////////////////////////
extern Klass* klass_Complex;
typedef struct {
  double real;
  double imag;
} Complex;
void val_to_complex_soft(Value v, double* real, double* imag);
Value complex_to_val(double real, double imag);
Complex* val_to_complex(Value v);
void init1_Complex();
void init2_Complex();

#endif
