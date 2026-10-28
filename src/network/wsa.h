/**
  \file   wsa.h
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  WSA procedures
*/

#ifndef WSA__H_
#define WSA__H_

#ifdef __cplusplus
namespace WSA {
extern "C" {
#endif

bool wsa_startup();
void wsa_cleanup();

int get_error();
const char *str_error(int err);

#ifdef __cplusplus
}
}
#endif


#endif // WSA__H_
