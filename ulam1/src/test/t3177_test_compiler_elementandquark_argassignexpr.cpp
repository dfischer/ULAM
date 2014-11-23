#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3177_test_compiler_elementandquark_argassignexpr)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(1);  Bar m_bar(0);  Bool(1) m_b(true);  Int(32) test() {  Foo f;  f m_bar ( )reset . . m_i ( m_b f m_bar val_b 1 [] . . = )check cast = m_i cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Bool m_b; Bool check(Bool b) { return b /* true */; } Int test() { Foo f;  f.m_bar.reset();  m_i = check(m_b = f.m_bar.val_b[1]); return m_i; } }\n"); //worked without f.m_bar as arg to check func; m_i not true? is m_bar f's? YES..now it works.

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset() { val_b[1] = true; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3177_test_compiler_elementandquark_argassignexpr)
  
} //end MFM

