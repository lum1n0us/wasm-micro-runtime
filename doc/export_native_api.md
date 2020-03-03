
Export native API to WASM application
=======================================================



Exporting native API steps
--------------------------

#### Step 1: Declare the function interface in WASM app

Create a in a header file for WASM app and declare the function that is exported from native. In this example, we declare foo and foo2 as below in header file example.h

```c
int  foo(int a, int b);
void foo2(char * msg, char * buffer, int buf_len);
```



#### Step 2: Define the native API

Define the native functions which are executed from the WASM app. The native function can be any name, for example **foo_native** and **foo2** here:

``` C
int foo_native(wasm_exec_env_t exec_env , int a, int b)
{
    return a+b;
}

void foo2(wasm_exec_env_t exec_env, char * msg, uint8 * buffer, int buf_len)
{
    strncpy(buffer, msg, buf_len);
}
```

The first argument exec_env must be defined using type **wasm_exec_env_t** which is the calling convention for exporting native API by WAMR . 

The rest arguments should be in the same types as the arguments of WASM function foo(), but there are a few special cases that are explained in section "Buffer address conversion and boundary check".  Regarding to the augment names, they don't have to be same, but we would suggest using same names for easy maintainence.



### Step 3: Register the native APIs

Register the native APIs in the runtime, then everything is fine.

``` C
// Define a array for the APIs to be exported. 
// Note: the array must be static defined since runtime
//       will keep it after registration
static NativeSymbol native_symbols[] = 
{
    {
        "foo", 		// the name of WASM function name
     	foo_native, // the native function pointer
        "(ii)i"		// the function prototype signature
    },
    {
        "foo2", 		// the name of WASM function name
     	foo2, 			// the native function pointer
        "($*~)"			// the function prototype signature
    }    
};


int n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
if (!wasm_runtime_register_natives("env",
                                   native_symbols, 
                                   n_native_symbols)) {
    goto fail1;
}

```

**Function signature**:

The function signature is a string for describing the function prototype.  It is critical to ensure the function signaure is correctly mapping the native function interface.

Each letter in the "()" represents a argument type, and the one following after ")" represents the return value type. The meanings of letters in function signature:

- 'i':  i32 
- 'I': i64 
- 'f': f32
- 'F': f64
- '*': the parameter is a buffer address in WASM application
- '~': the parameter is the length of WASM buffer as referred by preceding arugment "\*". It must follow after '*', otherwise registeration will fail
- '$': the parameter is a string in WASM application



# Call exported API in wasm application

Now we can call the exported native API in wasm application like this:
``` C
#include <stdio.h>
#include "example.h"   // where the APIs are declared

int main(int argc, char **argv)
{
    int a = 0, b = 1;
    char * msg = "hello";
    char buffer[100];

    int c = foo(a, b);   // call into native foo_native()
    foo2(msg, buffer, sizeof(buffer));   // call into native foo2()
    
    return 0;
}
```



## Buffer address conversion and boundary check

 A WebAssembly sandbox ensures applications only access to its own memory with a private address space. When passing a pointer address from WASM to native, address value must be converted to native address before the native function can access it. It is also the native world's responsibility to check the buffer length is not over its sandbox boundary. 



The signature letter '$', '\*' and '~' help the runtime do automatic address conversion and buffer boundary check, so the native function directly use the string and buffer address. **Notes**:  if '*' is not followed by '~', the native function should not assume the lengh of buffer is more than 1 byte.



As function parameters are always passed in 32 bits numbers, you can also use 'i' for the pointer type argument, then you must do all the address conversion and boundary checking in your native function. For example, if you change the foo2 signature  to "(iii)", then you will implement the native part as following:

```c
void foo2(wasm_exec_env_t exec_env, 
          uint32 msg_offset, 
          uint32 buffer_offset, 
          int32 buf_len)
{
    wasm_module_inst_t module_inst = get_module_inst(exec_env);
    char *buffer;
    char * msg ;

    // do boundary check
    if (!validate_app_str_addr(msg_offset))
        return 0;
    
    if (!validate_app_addr(buffer_offset, buf_len))
        return;

    // do address conversion
    buffer = addr_app_to_native(buffer_offset);
    msg = addr_app_to_native(msg_offset);

    strncpy(buffer, msg, buf_len);
}
```





## Security attention

The runtime builder should ensure not broking the memory sandbox when exporting the native function to WASM. 

A few key ground rules:

- Never pass any structure pointer to native (do data serialization instead)
- Do the pointer address conversion in the native API 
- Never pass function pointer to native 