// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME ex2aDict

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "mainwindow.h"
#include "calibrationwindow.h"

// Header files passed via #pragma extra_include

namespace ROOT {
   static TClass *MainWindow_Dictionary();
   static void MainWindow_TClassManip(TClass*);
   static void delete_MainWindow(void *p);
   static void deleteArray_MainWindow(void *p);
   static void destruct_MainWindow(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::MainWindow*)
   {
      ::MainWindow *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::MainWindow));
      static ::ROOT::TGenericClassInfo 
         instance("MainWindow", "mainwindow.h", 32,
                  typeid(::MainWindow), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &MainWindow_Dictionary, isa_proxy, 0,
                  sizeof(::MainWindow) );
      instance.SetDelete(&delete_MainWindow);
      instance.SetDeleteArray(&deleteArray_MainWindow);
      instance.SetDestructor(&destruct_MainWindow);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::MainWindow*)
   {
      return GenerateInitInstanceLocal((::MainWindow*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::MainWindow*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *MainWindow_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::MainWindow*)0x0)->GetClass();
      MainWindow_TClassManip(theClass);
   return theClass;
   }

   static void MainWindow_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrapper around operator delete
   static void delete_MainWindow(void *p) {
      delete ((::MainWindow*)p);
   }
   static void deleteArray_MainWindow(void *p) {
      delete [] ((::MainWindow*)p);
   }
   static void destruct_MainWindow(void *p) {
      typedef ::MainWindow current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::MainWindow

namespace {
  void TriggerDictionaryInitialization_ex2aDict_Impl() {
    static const char* headers[] = {
"mainwindow.h",
"calibrationwindow.h",
0
    };
    static const char* includePaths[] = {
"/home/brugi/Programmi/root-6.12.06/include",
"/home/brugi/Programmi/BeamSignals/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "ex2aDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate("$clingAutoload$mainwindow.h")))  MainWindow;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "ex2aDict dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "mainwindow.h"
#include "calibrationwindow.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"MainWindow", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("ex2aDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_ex2aDict_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_ex2aDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_ex2aDict() {
  TriggerDictionaryInitialization_ex2aDict_Impl();
}
