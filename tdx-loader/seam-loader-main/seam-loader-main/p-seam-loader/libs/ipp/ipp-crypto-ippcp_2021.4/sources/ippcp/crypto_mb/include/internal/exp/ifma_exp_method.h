/*******************************************************************************
* Copyright 2021 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef IFMA_EXP_METHOD_H
#define IFMA_EXP_METHOD_H

#include <crypto_mb/defs.h>

/* exponetiation processing window */
#define EXP_WIN_SIZE (5)
#define EXP_WIN_MASK ((1<<EXP_WIN_SIZE) -1)

/* exponentiation modulus range */
#define EXP_MODULUS_1024 (1024)
#define EXP_MODULUS_2048 (2048)
#define EXP_MODULUS_3072 (3072)
#define EXP_MODULUS_4096 (4096)
#define EXP_MODULUS_UNSUPPORT (0)

/* map exp modulus bit size to exp modulus slot  */
int bits_range(int bits);

/* exponentiation propotype */
typedef void(*exp_mb8)(int64u out[][8],
                 const int64u base[][8],
                 const int64u exponent[][8], int exp_bits,
                 const int64u modulus[][8],
                 const int64u toMont[][8],
                 const int64u k0_mb8[8],
                       int64u buffer[][8]);

/* exponentiations */
void ifma_modexp1024_mb(int64u out[][8],
                  const int64u base[][8],
                  const int64u exp[][8], int exp_bits,
                  const int64u modulus[][8],
                  const int64u toMont[][8],
                  const int64u k0[8],
                        int64u buffer[][8]);

void ifma_modexp2048_mb(int64u out[][8],
                  const int64u base[][8],
                  const int64u exp[][8], int exp_bits,
                  const int64u modulus[][8],
                  const int64u toMont[][8],
                  const int64u k0[8],
                        int64u buffer[][8]);

void ifma_modexp3072_mb(int64u out[][8],
                  const int64u base[][8],
                  const int64u exp[][8], int exp_bits,
                  const int64u modulus[][8],
                  const int64u toMont[][8],
                  const int64u k0[8],
                        int64u buffer[][8]);

void ifma_modexp4096_mb(int64u out[][8],
                  const int64u base[][8],
                  const int64u exp[][8], int exp_bits,
                  const int64u modulus[][8],
                  const int64u toMont[][8],
                  const int64u k0[8],
                        int64u buffer[][8]);

#endif /* IFMA_EXP_METHOD_H */
