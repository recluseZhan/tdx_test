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

static void sm4_ctr128_mask_kernel_mb16(__m512i* CTR, const __m512i* p_rk, __m512i loc_len, const int8u** loc_inp, int8u** loc_out, int8u* inc, __mmask16 tmp_mask, __mmask16 mb_mask)
{
    __m512i TMP[20];
    while (tmp_mask) {
        *CTR = IncBlock512(*CTR, inc);
        *(CTR + 1) = IncBlock512(*(CTR + 1), inc);
        *(CTR + 2) = IncBlock512(*(CTR + 2), inc);
        *(CTR + 3) = IncBlock512(*(CTR + 3), inc);
        TMP[0] = _mm512_shuffle_epi8(*CTR, M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(*(CTR + 1), M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(*(CTR + 2), M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(*(CTR + 3), M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[4], TMP[5], TMP[6], TMP[7], TMP[0], TMP[1], TMP[2], TMP[3]);

        *(CTR + 4) = IncBlock512(*(CTR + 4), inc);
        *(CTR + 5) = IncBlock512(*(CTR + 5), inc);
        *(CTR + 6) = IncBlock512(*(CTR + 6), inc);
        *(CTR + 7) = IncBlock512(*(CTR + 7), inc);
        TMP[0] = _mm512_shuffle_epi8(*(CTR + 4), M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(*(CTR + 5), M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(*(CTR + 6), M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(*(CTR + 7), M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[8], TMP[9], TMP[10], TMP[11], TMP[0], TMP[1], TMP[2], TMP[3]);

        *(CTR + 8) = IncBlock512(*(CTR + 8), inc);
        *(CTR + 9) = IncBlock512(*(CTR + 9), inc);
        *(CTR + 10) = IncBlock512(*(CTR + 10), inc);
        *(CTR + 11) = IncBlock512(*(CTR + 11), inc);
        TMP[0] = _mm512_shuffle_epi8(*(CTR + 8), M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(*(CTR + 9), M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(*(CTR + 10), M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(*(CTR + 11), M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[12], TMP[13], TMP[14], TMP[15], TMP[0], TMP[1], TMP[2], TMP[3]);

        *(CTR + 12) = IncBlock512(*(CTR + 12), inc);
        *(CTR + 13) = IncBlock512(*(CTR + 13), inc);
        *(CTR + 14) = IncBlock512(*(CTR + 14), inc);
        *(CTR + 15) = IncBlock512(*(CTR + 15), inc);
        TMP[0] = _mm512_shuffle_epi8(*(CTR + 12), M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(*(CTR + 13), M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(*(CTR + 14), M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(*(CTR + 15), M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[16], TMP[17], TMP[18], TMP[19], TMP[0], TMP[1], TMP[2], TMP[3]);

        for (int itr = 0, j = 0; itr < 8; itr++, j++) {
            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
        p_rk -= 32;

        /* Mask for data loading */
        __mmask64 stream_mask;
        int* p_loc_len = (int*)&loc_len;

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[4], TMP[5], TMP[6], TMP[7]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[0], stream_mask, _mm512_xor_si512(TMP[0], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[0])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[1], stream_mask, _mm512_xor_si512(TMP[1], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[1])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[2], stream_mask, _mm512_xor_si512(TMP[2], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[2])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[3], stream_mask, _mm512_xor_si512(TMP[3], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[3])));

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[8], TMP[9], TMP[10], TMP[11]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[4], stream_mask, _mm512_xor_si512(TMP[0], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[4])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[5], stream_mask, _mm512_xor_si512(TMP[1], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[5])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[6], stream_mask, _mm512_xor_si512(TMP[2], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[6])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[7], stream_mask, _mm512_xor_si512(TMP[3], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[7])));

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[12], TMP[13], TMP[14], TMP[15]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[8], stream_mask, _mm512_xor_si512(TMP[0], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[8])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[9], stream_mask, _mm512_xor_si512(TMP[1], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[9])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[10], stream_mask, _mm512_xor_si512(TMP[2], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[10])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[11], stream_mask, _mm512_xor_si512(TMP[3], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[11])));

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[16], TMP[17], TMP[18], TMP[19]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[12], stream_mask, _mm512_xor_si512(TMP[0], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[12])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[13], stream_mask, _mm512_xor_si512(TMP[1], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[13])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[14], stream_mask, _mm512_xor_si512(TMP[2], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[14])));
        UPDATE_STREAM_MASK_64(stream_mask, p_loc_len)
        _mm512_mask_storeu_epi8((__m512i*)loc_out[15], stream_mask, _mm512_xor_si512(TMP[3], _mm512_maskz_loadu_epi8(stream_mask, loc_inp[15])));

        /* Update pointers to data */
        M512(loc_inp) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_inp + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        M512(loc_out) = _mm512_add_epi64(_mm512_loadu_si512(loc_out), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_out + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_out + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        /* Update number of blocks left and processing mask */
        loc_len = _mm512_sub_epi32(loc_len, _mm512_set1_epi32(4 * SM4_BLOCK_SIZE));
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, loc_len, _mm512_set1_epi32(0), _MM_CMPINT_NLE);
        inc = (int8u*)nextInc;
    }

    /* clear local copy of sensitive data */
    zero_mb8((int64u(*)[8])TMP, sizeof(TMP) / sizeof(TMP[0]));
}

void sm4_ctr128_kernel_mb16(int8u* pa_out[SM4_LINES], const int8u* pa_inp[SM4_LINES], const int len[SM4_LINES], const int32u* key_sched[SM4_ROUNDS], __mmask16 mb_mask, int8u* pa_ctr[SM4_LINES])
{
    const int8u* loc_inp[SM4_LINES];
    int8u* loc_out[SM4_LINES];

    /* Create the local copy of the input data length in bytes and set it to zero for non-valid buffers */
    __m512i loc_len;
    loc_len = _mm512_loadu_si512(len);
    loc_len = _mm512_mask_set1_epi32(loc_len, ~mb_mask, 0);

    /* Local copies of the pointers to input and otput buffers */
    _mm512_storeu_si512((void*)loc_inp, _mm512_loadu_si512(pa_inp));
    _mm512_storeu_si512((void*)(loc_inp + 8), _mm512_loadu_si512(pa_inp + 8));

    _mm512_storeu_si512(loc_out, _mm512_loadu_si512(pa_out));
    _mm512_storeu_si512(loc_out + 8, _mm512_loadu_si512(pa_out + 8));

    /* Pointer p_rk is set to the beginning of the key schedule */
    const __m512i* p_rk = (const __m512i*)key_sched;

    /* TMP[] - temporary buffer for processing */
    /* CTR - store CTR values                  */
    __m512i TMP[20];
    __m512i CTR[16];

    /* Load CTR value from valid buffers */
    mb_mask = _mm512_mask_cmp_epi32_mask(mb_mask, loc_len, _mm512_setzero_si512(), _MM_CMPINT_NLE);
    for (int i = 0; i < SM4_LINES; i++) {
        if (0x1 & (mb_mask >> i)) {
            CTR[i] = _mm512_broadcast_i64x2(_mm_loadu_si128((__m128i*)pa_ctr[i]));
            /* Read string counter and convert to numerical */
            CTR[i] = _mm512_shuffle_epi8(CTR[i], M512(swapEndianness));
        }
        else
            CTR[i] = _mm512_setzero_si512();
    }

    /* Generate the mask to process 4 blocks from each buffer */
    __mmask16 tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, loc_len, _mm512_set1_epi32(4 * SM4_BLOCK_SIZE), _MM_CMPINT_NLT);

    int8u* inc = (int8u*)firstInc;

    /* Go to this loop if all 16 buffers contain at least 4 blocks each */
    while (tmp_mask == 0xFFFF) {
        CTR[0] = IncBlock512(CTR[0], inc);
        CTR[1] = IncBlock512(CTR[1], inc);
        CTR[2] = IncBlock512(CTR[2], inc);
        CTR[3] = IncBlock512(CTR[3], inc);
        TMP[0] = _mm512_shuffle_epi8(CTR[0], M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(CTR[1], M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(CTR[2], M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(CTR[3], M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[4], TMP[5], TMP[6], TMP[7], TMP[0], TMP[1], TMP[2], TMP[3]);

        CTR[4] = IncBlock512(CTR[4], inc);
        CTR[5] = IncBlock512(CTR[5], inc);
        CTR[6] = IncBlock512(CTR[6], inc);
        CTR[7] = IncBlock512(CTR[7], inc);
        TMP[0] = _mm512_shuffle_epi8(CTR[4], M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(CTR[5], M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(CTR[6], M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(CTR[7], M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[8], TMP[9], TMP[10], TMP[11], TMP[0], TMP[1], TMP[2], TMP[3]);

        CTR[8] = IncBlock512(CTR[8], inc);
        CTR[9] = IncBlock512(CTR[9], inc);
        CTR[10] = IncBlock512(CTR[10], inc);
        CTR[11] = IncBlock512(CTR[11], inc);
        TMP[0] = _mm512_shuffle_epi8(CTR[8], M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(CTR[9], M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(CTR[10], M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(CTR[11], M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[12], TMP[13], TMP[14], TMP[15], TMP[0], TMP[1], TMP[2], TMP[3]);

        CTR[12] = IncBlock512(CTR[12], inc);
        CTR[13] = IncBlock512(CTR[13], inc);
        CTR[14] = IncBlock512(CTR[14], inc);
        CTR[15] = IncBlock512(CTR[15], inc);
        TMP[0] = _mm512_shuffle_epi8(CTR[12], M512(swapWordsOrder));
        TMP[1] = _mm512_shuffle_epi8(CTR[13], M512(swapWordsOrder));
        TMP[2] = _mm512_shuffle_epi8(CTR[14], M512(swapWordsOrder));
        TMP[3] = _mm512_shuffle_epi8(CTR[15], M512(swapWordsOrder));
        TRANSPOSE_INP_512(TMP[16], TMP[17], TMP[18], TMP[19], TMP[0], TMP[1], TMP[2], TMP[3]);

        for (int itr = 0, j = 0; itr < 8; itr++, j++) {
            /* initial xors */
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
            EXPAND_ONE_RKEY(TMP, p_rk);  p_rk++;
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
        p_rk -= 32;

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[4], TMP[5], TMP[6], TMP[7]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)loc_out[0], _mm512_xor_si512(TMP[0], _mm512_loadu_si512(loc_inp[0])));
        _mm512_storeu_si512((__m512i*)loc_out[1], _mm512_xor_si512(TMP[1], _mm512_loadu_si512(loc_inp[1])));
        _mm512_storeu_si512((__m512i*)loc_out[2], _mm512_xor_si512(TMP[2], _mm512_loadu_si512(loc_inp[2])));
        _mm512_storeu_si512((__m512i*)loc_out[3], _mm512_xor_si512(TMP[3], _mm512_loadu_si512(loc_inp[3])));

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[8], TMP[9], TMP[10], TMP[11]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)loc_out[4], _mm512_xor_si512(TMP[0], _mm512_loadu_si512(loc_inp[4])));
        _mm512_storeu_si512((__m512i*)loc_out[5], _mm512_xor_si512(TMP[1], _mm512_loadu_si512(loc_inp[5])));
        _mm512_storeu_si512((__m512i*)loc_out[6], _mm512_xor_si512(TMP[2], _mm512_loadu_si512(loc_inp[6])));
        _mm512_storeu_si512((__m512i*)loc_out[7], _mm512_xor_si512(TMP[3], _mm512_loadu_si512(loc_inp[7])));

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[12], TMP[13], TMP[14], TMP[15]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)loc_out[8], _mm512_xor_si512(TMP[0], _mm512_loadu_si512(loc_inp[8])));
        _mm512_storeu_si512((__m512i*)loc_out[9], _mm512_xor_si512(TMP[1], _mm512_loadu_si512(loc_inp[9])));
        _mm512_storeu_si512((__m512i*)loc_out[10], _mm512_xor_si512(TMP[2], _mm512_loadu_si512(loc_inp[10])));
        _mm512_storeu_si512((__m512i*)loc_out[11], _mm512_xor_si512(TMP[3], _mm512_loadu_si512(loc_inp[11])));

        TRANSPOSE_OUT_512(TMP[0], TMP[1], TMP[2], TMP[3], TMP[16], TMP[17], TMP[18], TMP[19]);
        TMP[0] = _mm512_shuffle_epi8(TMP[0], M512(swapBytes));
        TMP[1] = _mm512_shuffle_epi8(TMP[1], M512(swapBytes));
        TMP[2] = _mm512_shuffle_epi8(TMP[2], M512(swapBytes));
        TMP[3] = _mm512_shuffle_epi8(TMP[3], M512(swapBytes));
        _mm512_storeu_si512((__m512i*)loc_out[12], _mm512_xor_si512(TMP[0], _mm512_loadu_si512(loc_inp[12])));
        _mm512_storeu_si512((__m512i*)loc_out[13], _mm512_xor_si512(TMP[1], _mm512_loadu_si512(loc_inp[13])));
        _mm512_storeu_si512((__m512i*)loc_out[14], _mm512_xor_si512(TMP[2], _mm512_loadu_si512(loc_inp[14])));
        _mm512_storeu_si512((__m512i*)loc_out[15], _mm512_xor_si512(TMP[3], _mm512_loadu_si512(loc_inp[15])));

        /* Update pointers to data */
        M512(loc_inp) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_inp + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_inp + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        M512(loc_out) = _mm512_add_epi64(_mm512_loadu_si512(loc_out), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));
        M512(loc_out + 8) = _mm512_add_epi64(_mm512_loadu_si512(loc_out + 8), _mm512_set1_epi64(4 * SM4_BLOCK_SIZE));

        /* Update number of blocks left and processing mask */
        loc_len = _mm512_sub_epi32(loc_len, _mm512_set1_epi32(4 * SM4_BLOCK_SIZE));
        tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, loc_len, _mm512_set1_epi32(4 * SM4_BLOCK_SIZE), _MM_CMPINT_NLT);
        inc = (int8u*)nextInc;
    }

    /* Check if we have any data */
    tmp_mask = _mm512_mask_cmp_epi32_mask(mb_mask, loc_len, _mm512_setzero_si512(), _MM_CMPINT_NLE);
    if (tmp_mask)
        sm4_ctr128_mask_kernel_mb16(CTR, p_rk, loc_len, loc_inp, loc_out, inc, tmp_mask, mb_mask);

    /* Save counters */
    for (int i = 0; i < SM4_LINES; i++) {
        if (0x1 & (mb_mask >> i)) {
            CTR[i] = _mm512_shuffle_epi8(CTR[i], M512(swapEndianness));
            _mm_storeu_si128((__m128i*)pa_ctr[i], _mm512_castsi512_si128(CTR[i]));
            CTR[i] = _mm512_setzero_si512();
        }
    }

    /* clear local copy of sensitive data */
    zero_mb8((int64u(*)[8])TMP, sizeof(TMP) / sizeof(TMP[0]));
}
