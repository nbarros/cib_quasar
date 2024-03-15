/*
 * test_sizes.cpp
 *
 *  Created on: Mar 15, 2024
 *      Author: Nuno Barros
 */

#include <cstdio>
#include <cstdint>

int main(int argc, char**argv)
{
  printf("Size of int         : %u\n", sizeof(int));
  printf("Size of void        : %u\n", sizeof(void));
  printf("Size of void*       : %u\n", sizeof(void*));
  printf("Size of uintptr_t   : %u\n", sizeof(uintptr_t));
  printf("Size of uintptr_t*  : %u\n", sizeof(uintptr_t*));
  printf("Size of intptr_t    : %u\n", sizeof(intptr_t));
  printf("Size of uint32_t    : %u\n", sizeof(uint32_t));
  printf("Size of uint64_t    : %u\n", sizeof(uint64_t));
  return 0;
}

