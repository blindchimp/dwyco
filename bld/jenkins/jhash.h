
/*
 * $Header: /e/linux/home/dwight/repo/jenkins/rcs/jhash.h 1.1 1998/06/22 02:26:58 dwight Exp $
 */
#ifndef JHASH_H
#define JHASH_H
#ifdef _MSC_VER
#include "msint.h"
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

unsigned int jenkins_hash(char *k, int len);
uint32_t jenkins_hashlittle( const void *key, size_t length, uint32_t initval);
#ifdef __cplusplus
};
#endif

#endif
