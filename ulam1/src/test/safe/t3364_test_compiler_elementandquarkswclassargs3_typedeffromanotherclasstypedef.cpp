#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3364_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_R { P(3) pvar( constant Int(32) a = 3;  Bool(3) b(false); );  Int(32) test() {  3u cast return } }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef P(a) Foo;  <NOMAIN> }\nUq_P { constant Int(32) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3345
      // recursive typedefs as datamember variable type (requires quark, unless EP)
      // must already be parsed!
      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nelement R {\nV.Woof.Foo pvar;\n Int test() {\n return pvar.sizeof;\n}\n}\n");

      bool rtn1 = fms->add("P.ulam","ulam 1;\nquark P(Int a) {\nBool(a) b;\n}\n");

      // as primitive, as regular class, we have no problems.
      bool rtn2 = fms->add("Q.ulam","ulam 1;\nuse P;\nquark Q(Int s) {\ntypedef P(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3364_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)

} //end MFM
