#ifndef _PREREQS_OCI_H_
#define _PREREQS_OCI_H_ "$Revision: 412.1 $ $Date: 2004/11/18 10:27:35 $"
#include "versions.h"
HEADER_VERSION(_PREREQS_OCI_H_);

#define __OCI_VERSION__ 901
#define __STRING__OCI_VERSION__ "901"

#if (__LIBTI_VERSION__ < __OCI_VERSION__)
#error "LIBTI VERSION " __STRINGOF__LIBTI_VERSION__ " IS LESS OCI VERSION " __STRING__OCI_VERSION__
#endif

#endif

