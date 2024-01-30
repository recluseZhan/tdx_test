﻿/*******************************************************************************
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

#include <internal/sm4/sm4_mb.h>
#include <internal/rsa/ifma_rsa_arith.h>

void sm4_ecb_kernel_mb16(int8u* pa_out[SM4_LINES], const int8u* pa_inp[SM4_LINES], const int len[SM4_LINES], const int32u* key_sched[SM4_ROUNDS], __mmask16 mb_mask, int operation)
{
    const int8u* loc_inp[SM4_LINES];
    int8u* loc_out[SM4_LINES];

    /* Length of the input data in 128-bit chunks - number of SM4 blocks */
    __m512i num_blocks;
    GET_NUM_BLOCKS(num_blocks, len, SM4_BLOCK_SIZE);

    /* Local copies of the pointers to input and otput buffers */
    _mm512_storeu_si512((void*)loc_inp, _mm512_loadu_si512(pa_inp));
    _mm512_storeu_si512((void*)(loc_inp + 8), _mm512_loadu_si512(pa_inp + 8));

    _mm512_storeu_si512(loc_out, _mm512_loadu_si512(pa_out));
    _mm512_storeu_si512(loc_out + 8, _mm512_loadu_si512(pa_out + 8));

    /* Depending on the operation(enc or dec): sign allows to go up and down on the key schedule */
    /* p_rk set to the beginning or to the end of the key schedule */
    const int sign = (operation == SM4_ENC) ? 1 : -1;
    const __m512i* p_rk = (operation == SM4_ENC) ? (const __m512i*)key_sched : ((const __m512i*)key_sched + (SM4_ROUNDS - 1));

    __m512i TMP[20];

    /* Generate the mask to process 4 blocks from each buffer */
    __mmask16 tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_set1_epi32(4), _MM_CMPINT_NLT);

    /* Go to this loop if all 16 buffers contain at least 4 blocks each */
    while (tmp_mask == 0xFFFF) {
        TMP[0] = _mm512_loadu_si512(loc_inp[0]);
        TMP[1] = _mm512_loadu_si512((__m512i*)(loc_inp[1]));
        TMP[2] = _mm512_loadu_si512((__m512i*)(loc_inp[2]));
        TMP[3] = _mm512_loadu_si512((__m512i*)(loc_inp[3]));
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[4], TMP[5], TMP[6], TMP[7], TMP[0], TMP[1], TMP[2], TMP[3]);

        TMP[0] = _mm512_loadu_si512((__m512i*)(loc_inp[4]));
        TMP[1] = _mm512_loadu_si512((__m512i*)(loc_inp[5]));
        TMP[2] = _mm512_loadu_si512((__m512i*)(loc_inp[6]));
        TMP[3] = _mm512_loadu_si512((__m512i*)(loc_inp[7]));
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[8], TMP[9], TMP[10], TMP[11], TMP[0], TMP[1], TMP[2], TMP[3]);

        TMP[0] = _mm512_loadu_si512((__m512i*)(loc_inp[8]));
        TMP[1] = _mm512_loadu_si512((__m512i*)(loc_inp[9]));
        TMP[2] = _mm512_loadu_si512((__m512i*)(loc_inp[10]));
        TMP[3] = _mm512_loadu_si512((__m512i*)(loc_inp[11]));
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[12], TMP[13], TMP[14], TMP[15], TMP[0], TMP[1], TMP[2], TMP[3]);

        TMP[0] = _mm512_loadu_si512((__m512i*)(loc_inp[12]));
        TMP[1] = _mm512_loadu_si512((__m512i*)(loc_inp[13]));
        TMP[2] = _mm512_loadu_si512((__m512i*)(loc_inp[14]));
        TMP[3] = _mm512_loadu_si512((__m512i*)(loc_inp[15]));
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[16], TMP[17], TMP[18], TMP[19], TMP[0], TMP[1], TMP[2], TMP[3]);

        for (int itr = 0, j = 0; itr < 8; itr++, j++) {
            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[5]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[6]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[7]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[9]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[10]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[11]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[13]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[14]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[15]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[17]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[18]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[19]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[4] = _mm512_xor_si512(_mm512_xor_si512(TMP[4], TMP[0]), Lblock512(TMP[0]));
            TMP[8] = _mm512_xor_si512(_mm512_xor_si512(TMP[8], TMP[1]), Lblock512(TMP[1]));
            TMP[12] = _mm512_xor_si512(_mm512_xor_si512(TMP[12], TMP[2]), Lblock512(TMP[2]));
            TMP[16] = _mm512_xor_si512(_mm512_xor_si512(TMP[16], TMP[3]), Lblock512(TMP[3]));

            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[6]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[7]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[4]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[10]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[11]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[8]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[14]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[15]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[12]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[18]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[19]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[16]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[5] = _mm512_xor_si512(_mm512_xor_si512(TMP[5], TMP[0]), Lblock512(TMP[0]));
            TMP[9] = _mm512_xor_si512(_mm512_xor_si512(TMP[9], TMP[1]), Lblock512(TMP[1]));
            TMP[13] = _mm512_xor_si512(_mm512_xor_si512(TMP[13], TMP[2]), Lblock512(TMP[2]));
            TMP[17] = _mm512_xor_si512(_mm512_xor_si512(TMP[17], TMP[3]), Lblock512(TMP[3]));

            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[7]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[4]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[5]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[11]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[8]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[9]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[15]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[12]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[13]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[19]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[16]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[17]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[6] = _mm512_xor_si512(_mm512_xor_si512(TMP[6], TMP[0]), Lblock512(TMP[0]));
            TMP[10] = _mm512_xor_si512(_mm512_xor_si512(TMP[10], TMP[1]), Lblock512(TMP[1]));
            TMP[14] = _mm512_xor_si512(_mm512_xor_si512(TMP[14], TMP[2]), Lblock512(TMP[2]));
            TMP[18] = _mm512_xor_si512(_mm512_xor_si512(TMP[18], TMP[3]), Lblock512(TMP[3]));

            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[4]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[5]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[6]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[8]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[9]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[10]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[12]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[13]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[14]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[16]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[17]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[18]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[7] = _mm512_xor_si512(_mm512_xor_si512(TMP[7], TMP[0]), Lblock512(TMP[0]));
            TMP[11] = _mm512_xor_si512(_mm512_xor_si512(TMP[11], TMP[1]), Lblock512(TMP[1]));
            TMP[15] = _mm512_xor_si512(_mm512_xor_si512(TMP[15], TMP[2]), Lblock512(TMP[2]));
            TMP[19] = _mm512_xor_si512(_mm512_xor_si512(TMP[19], TMP[3]), Lblock512(TMP[3]));
        }
        p_rk -= sign*32;

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[4], TMP[5], TMP[6], TMP[7]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)(loc_out[0]), TMP[0]);
        _mm512_storeu_si512((__m512i*)(loc_out[1]), TMP[1]);
        _mm512_storeu_si512((__m512i*)(loc_out[2]), TMP[2]);
        _mm512_storeu_si512((__m512i*)(loc_out[3]), TMP[3]);

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[8], TMP[9], TMP[10], TMP[11]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)(loc_out[4]), TMP[0]);
        _mm512_storeu_si512((__m512i*)(loc_out[5]), TMP[1]);
        _mm512_storeu_si512((__m512i*)(loc_out[6]), TMP[2]);
        _mm512_storeu_si512((__m512i*)(loc_out[7]), TMP[3]);

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[12], TMP[13], TMP[14], TMP[15]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)(loc_out[8]), TMP[0]);
        _mm512_storeu_si512((__m512i*)(loc_out[9]), TMP[1]);
        _mm512_storeu_si512((__m512i*)(loc_out[10]), TMP[2]);
        _mm512_storeu_si512((__m512i*)(loc_out[11]), TMP[3]);

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[16], TMP[17], TMP[18], TMP[19]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)(loc_out[12]), TMP[0]);
        _mm512_storeu_si512((__m512i*)(loc_out[13]), TMP[1]);
        _mm512_storeu_si512((__m512i*)(loc_out[14]), TMP[2]);
        _mm512_storeu_si512((__m512i*)(loc_out[15]), TMP[3]);

        /* Update pointers to data */
        M512(loc_inp) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_inp + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        M512(loc_out) = _mm512_add_epi64(_mm512_loadu_si512(loc_out), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_out + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_out + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        /* Update number of blocks left and processing mask */
        num_blocks = _mm512_sub_epi32(num_blocks, _mm512_set1_epi32(4));
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_set1_epi32(4), _MM_CMPINT_NLT);
    }

    /* Check if we have any data */
    tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_setzero_si512(), _MM_CMPINT_NLE);

    while (tmp_mask) {
        /* Generate the array of masks for data loading. 0 - 4 bloks will be can load from each buffer - depend on the amount of remaining data */
        __mmask8 block_mask[SM4_LINES];

        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_set1_epi32(4), _MM_CMPINT_NLT);
        /* Will be loaded 4 blocks of data */
        M128(block_mask) = _mm_maskz_set1_epi8(tmp_mask, 0xFF);
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_set1_epi32(3), _MM_CMPINT_EQ);
        /* Will be loaded 3 blocks of data */
        M128(block_mask) = _mm_mask_set1_epi8(M128(block_mask), tmp_mask, 0x3F);
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_set1_epi32(2), _MM_CMPINT_EQ);
        /* Will be loaded 2 blocks of data */
        M128(block_mask) = _mm_mask_set1_epi8(M128(block_mask), tmp_mask, 0xF);
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_set1_epi32(1), _MM_CMPINT_EQ);
        /* Will be loaded 1 block of data */
        M128(block_mask) = _mm_mask_set1_epi8(M128(block_mask), tmp_mask, 0x3);

        TMP[0] = _mm512_maskz_loadu_epi64(block_mask[0], loc_inp[0]);
        TMP[1] = _mm512_maskz_loadu_epi64(block_mask[1], loc_inp[1]);
        TMP[2] = _mm512_maskz_loadu_epi64(block_mask[2], loc_inp[2]);
        TMP[3] = _mm512_maskz_loadu_epi64(block_mask[3], loc_inp[3]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[4], TMP[5], TMP[6], TMP[7], TMP[0], TMP[1], TMP[2], TMP[3]);

        TMP[0] = _mm512_maskz_loadu_epi64(block_mask[4], loc_inp[4]);
        TMP[1] = _mm512_maskz_loadu_epi64(block_mask[5], loc_inp[5]);
        TMP[2] = _mm512_maskz_loadu_epi64(block_mask[6], loc_inp[6]);
        TMP[3] = _mm512_maskz_loadu_epi64(block_mask[7], loc_inp[7]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[8], TMP[9], TMP[10], TMP[11], TMP[0], TMP[1], TMP[2], TMP[3]);

        TMP[0] = _mm512_maskz_loadu_epi64(block_mask[8], loc_inp[8]);
        TMP[1] = _mm512_maskz_loadu_epi64(block_mask[9], loc_inp[9]);
        TMP[2] = _mm512_maskz_loadu_epi64(block_mask[10], loc_inp[10]);
        TMP[3] = _mm512_maskz_loadu_epi64(block_mask[11], loc_inp[11]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[12], TMP[13], TMP[14], TMP[15], TMP[0], TMP[1], TMP[2], TMP[3]);

        TMP[0] = _mm512_maskz_loadu_epi64(block_mask[12], loc_inp[12]);
        TMP[1] = _mm512_maskz_loadu_epi64(block_mask[13], loc_inp[13]);
        TMP[2] = _mm512_maskz_loadu_epi64(block_mask[14], loc_inp[14]);
        TMP[3] = _mm512_maskz_loadu_epi64(block_mask[15], loc_inp[15]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        TRANSPOSE_INP_512(TMP[16], TMP[17], TMP[18], TMP[19], TMP[0], TMP[1], TMP[2], TMP[3]);

        for (int itr = 0, j = 0; itr < 8; itr++, j++) {
            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[5]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[6]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[7]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[9]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[10]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[11]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[13]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[14]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[15]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[17]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[18]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[19]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[4] = _mm512_xor_si512(_mm512_xor_si512(TMP[4], TMP[0]), Lblock512(TMP[0]));
            TMP[8] = _mm512_xor_si512(_mm512_xor_si512(TMP[8], TMP[1]), Lblock512(TMP[1]));
            TMP[12] = _mm512_xor_si512(_mm512_xor_si512(TMP[12], TMP[2]), Lblock512(TMP[2]));
            TMP[16] = _mm512_xor_si512(_mm512_xor_si512(TMP[16], TMP[3]), Lblock512(TMP[3]));

            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[6]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[7]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[4]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[10]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[11]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[8]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[14]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[15]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[12]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[18]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[19]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[16]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[5] = _mm512_xor_si512(_mm512_xor_si512(TMP[5], TMP[0]), Lblock512(TMP[0]));
            TMP[9] = _mm512_xor_si512(_mm512_xor_si512(TMP[9], TMP[1]), Lblock512(TMP[1]));
            TMP[13] = _mm512_xor_si512(_mm512_xor_si512(TMP[13], TMP[2]), Lblock512(TMP[2]));
            TMP[17] = _mm512_xor_si512(_mm512_xor_si512(TMP[17], TMP[3]), Lblock512(TMP[3]));

            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[7]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[4]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[5]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[11]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[8]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[9]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[15]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[12]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[13]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[19]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[16]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[17]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[6] = _mm512_xor_si512(_mm512_xor_si512(TMP[6], TMP[0]), Lblock512(TMP[0]));
            TMP[10] = _mm512_xor_si512(_mm512_xor_si512(TMP[10], TMP[1]), Lblock512(TMP[1]));
            TMP[14] = _mm512_xor_si512(_mm512_xor_si512(TMP[14], TMP[2]), Lblock512(TMP[2]));
            TMP[18] = _mm512_xor_si512(_mm512_xor_si512(TMP[18], TMP[3]), Lblock512(TMP[3]));

            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk += sign * 1;
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[4]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[5]);
            TMP[0] = _mm512_xor_si512(TMP[0], TMP[6]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[8]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[9]);
            TMP[1] = _mm512_xor_si512(TMP[1], TMP[10]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[12]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[13]);
            TMP[2] = _mm512_xor_si512(TMP[2], TMP[14]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[16]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[17]);
            TMP[3] = _mm512_xor_si512(TMP[3], TMP[18]);
            /* Sbox */
            TMP[0] = sBox512(TMP[0]);
            TMP[1] = sBox512(TMP[1]);
            TMP[2] = sBox512(TMP[2]);
            TMP[3] = sBox512(TMP[3]);
            /* Sbox done, now L */
            TMP[7] = _mm512_xor_si512(_mm512_xor_si512(TMP[7], TMP[0]), Lblock512(TMP[0]));
            TMP[11] = _mm512_xor_si512(_mm512_xor_si512(TMP[11], TMP[1]), Lblock512(TMP[1]));
            TMP[15] = _mm512_xor_si512(_mm512_xor_si512(TMP[15], TMP[2]), Lblock512(TMP[2]));
            TMP[19] = _mm512_xor_si512(_mm512_xor_si512(TMP[19], TMP[3]), Lblock512(TMP[3]));
        }
        p_rk -= sign * 32;

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[4], TMP[5], TMP[6], TMP[7]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_mask_storeu_epi64(loc_out[0], block_mask[0], TMP[0]);
        _mm512_mask_storeu_epi64(loc_out[1], block_mask[1], TMP[1]);
        _mm512_mask_storeu_epi64(loc_out[2], block_mask[2], TMP[2]);
        _mm512_mask_storeu_epi64(loc_out[3], block_mask[3], TMP[3]);

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[8], TMP[9], TMP[10], TMP[11]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_mask_storeu_epi64(loc_out[4], block_mask[4], TMP[0]);
        _mm512_mask_storeu_epi64(loc_out[5], block_mask[5], TMP[1]);
        _mm512_mask_storeu_epi64(loc_out[6], block_mask[6], TMP[2]);
        _mm512_mask_storeu_epi64(loc_out[7], block_mask[7], TMP[3]);

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[12], TMP[13], TMP[14], TMP[15]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_mask_storeu_epi64(loc_out[8], block_mask[8], TMP[0]);
        _mm512_mask_storeu_epi64(loc_out[9], block_mask[9], TMP[1]);
        _mm512_mask_storeu_epi64(loc_out[10], block_mask[10], TMP[2]);
        _mm512_mask_storeu_epi64(loc_out[11], block_mask[11], TMP[3]);

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[16], TMP[17], TMP[18], TMP[19]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_mask_storeu_epi64(loc_out[12], block_mask[12], TMP[0]);
        _mm512_mask_storeu_epi64(loc_out[13], block_mask[13], TMP[1]);
        _mm512_mask_storeu_epi64(loc_out[14], block_mask[14], TMP[2]);
        _mm512_mask_storeu_epi64(loc_out[15], block_mask[15], TMP[3]);

        /* Update pointers to data */
        M512(loc_inp) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_inp + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        M512(loc_out) = _mm512_add_epi64(_mm512_loadu_si512(loc_out), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_out + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_out + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        /* Update the number of blocks. For some buffers, the value can become zero or a negative number - these buffers will not be processed  */
        num_blocks = _mm512_sub_epi32(num_blocks, _mm512_set1_epi32(4));

        /* Check if we have any data */
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, num_blocks, _mm512_setzero_si512(), _MM_CMPINT_NLE);
    }

    /* clear local copy of sensitive data */
    zero_mb8((int64u(*)[8])TMP, sizeof(TMP) / sizeof(TMP[0]));
}
