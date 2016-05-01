/* -*- C++ -*- */

#include "UlamDefs.h"
#include "MDist.h"

namespace MFM {

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9212getLastIndex(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131u<EC> Uv_6radius) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_10161u<EC> ret(MDist<R>::get().GetLastIndex(radius));
    return ret;
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9213getFirstIndex(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131u<EC> Uv_6radius) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_10161u<EC> ret(MDist<R>::get().GetFirstIndex(radius));
    return ret;
  }
} //MFM
