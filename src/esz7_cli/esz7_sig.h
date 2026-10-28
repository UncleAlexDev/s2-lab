/**
  \file   sig.h
  \author Aleksey Lekhtin
  \date   April 4, 2024
  \brief  catch signals
*/

#ifndef SIGNALS__H
#define SIGNALS__H

#include <stdbool.h>

#ifdef __cplusplus
namespace ESZ7_SIGNAL {
extern "C" {
#endif

bool esz7_set_sig_action();

#ifdef __cplusplus
} // extern C
} // namespace ESZ7_SIGNAL
#endif


#endif // SIGNALS__H
