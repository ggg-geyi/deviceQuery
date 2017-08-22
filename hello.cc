#include <node.h>
#include <v8.h>
#include <iostream>
#include <windows.h>
#include "WMI_DeviceQuery.h"

using namespace v8;
using namespace node;
using namespace std;
  
void Method(const v8::FunctionCallbackInfo<v8::Value>& args) {  
  Isolate* isolate = Isolate::GetCurrent();    


    // 参数长度判断 
    if (args.Length() < 1) { 
        isolate->ThrowException(Exception::TypeError( 
            String::NewFromUtf8(isolate, "Wrong number of arguments"))); 
        return; 
    } 
 
    // 参数类型判断 
    if (!args[0]->IsNumber()) { 
        //抛出错误 
        isolate->ThrowException(Exception::TypeError( 
            String::NewFromUtf8(isolate, "argumnets must be number"))); 
    } 

  int type = args[0]->NumberValue();
  T_DEVICE_PROPERTY properties;
  WMI_DeviceQuery(type,&properties,1);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, properties.szProperty));  
}  
  
void init(Handle<Object> exports) {  
  NODE_SET_METHOD(exports, "hello", Method);  
}  
  
NODE_MODULE(addon, init)
