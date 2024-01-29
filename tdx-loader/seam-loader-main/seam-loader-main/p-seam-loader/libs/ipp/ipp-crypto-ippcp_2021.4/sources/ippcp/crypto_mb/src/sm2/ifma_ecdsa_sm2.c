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

#include <crypto_mb/status.h>
#include <crypto_mb/ec_sm2.h>

#include <internal/common/ifma_defs.h>
#include <internal/common/ifma_cvt52.h>
#include <internal/sm2/ifma_ecpoint_sm2.h>
#include <internal/sm3/sm3_mb8.h>
#include <internal/rsa/ifma_rsa_arith.h>

#ifndef BN_OPENSSL_DISABLE
#include <openssl/bn.h>
#include <openssl/ec.h>
#ifdef OPENSSL_IS_BORINGSSL
#include <openssl/ecdsa.h>
#endif
#endif

/* constants for Z digest computation */
/* EC SM2 equation coeficient a, big endian */
static const int8u a[]  = "\xFF\xFF\xFF\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC";
/* EC SM2 equation coeficient b, big endian */
static const int8u b[]  = "\x28\xE9\xFA\x9E\x9D\x9F\x5E\x34\x4D\x5A\x9E\x4B\xCF\x65\x09\xA7\xF3\x97\x89\xF5\x15\xAB\x8F\x92\xDD\xBC\xBD\x41\x4D\x94\x0E\x93";
/* x coordinate of the EC SM2 generator point in affine coordinates, big endian */
static const int8u xG[] = "\x32\xC4\xAE\x2C\x1F\x19\x81\x19\x5F\x99\x04\x46\x6A\x39\xC9\x94\x8F\xE3\x0B\xBF\xF2\x66\x0B\xE1\x71\x5A\x45\x89\x33\x4C\x74\xC7";
/* y coordinate of the EC SM2 generator point in affine coordinates, big endian */
static const int8u yG[] = "\xBC\x37\x36\xA2\xF4\xF6\x77\x9C\x59\xBD\xCE\xE3\x6B\x69\x21\x53\xD0\xA9\x87\x7C\xC6\x2A\x47\x40\x02\xDF\x32\xE5\x21\x39\xF0\xA0";

static const int8u * pa_a[]  = {a, a, a, a, a, a, a, a };
static const int8u * pa_b[]  = {b, b, b, b, b, b, b, b };
static const int8u * pa_xG[] = {xG,xG,xG,xG,xG,xG,xG,xG};
static const int8u * pa_yG[] = {yG,yG,yG,yG,yG,yG,yG,yG};

static int len_1[8]  = {1, 1, 1, 1, 1, 1, 1, 1 };
static int len_32[8] = {32,32,32,32,32,32,32,32};

/*
// common functions
*/

/* compute Z digest = SM3( ENTL || ID || a || b ||xG || yG || xA || yA ) */
static void sm2_ecdsa_compute_z_digest(int8u* pa_z_digest[8],
                                 const int8u* pa_user_id[8],
                                    const int user_id_len[8],
                                 const int8u* pa_pubx[8],
                                 const int8u* pa_puby[8])
{
   SM3_CTX_mb8 ctx;
   SM3_CTX_mb8* p_ctx = &ctx;

   sm3_init_mb8(p_ctx);

   int8u entl_lo[8];
   int8u entl_hi[8];

   int8u * pa_entl_lo[8];
   int8u * pa_entl_hi[8];

   for (int i = 0; i < 8; i++)
   {
      int entl = ((user_id_len[i] * 8) & 0xFFFF);

      entl_lo[i] = entl & 0xFF;
      entl_hi[i] = entl >> 8;

      pa_entl_lo[i] = &entl_lo[i];
      pa_entl_hi[i] = &entl_hi[i];
   }

   sm3_update_mb8((const int8u **)pa_entl_hi, len_1, p_ctx);
   sm3_update_mb8((const int8u **)pa_entl_lo, len_1, p_ctx);

   sm3_update_mb8((const int8u **)pa_user_id, (int *)user_id_len, p_ctx);

   sm3_update_mb8((const int8u **)pa_a,  len_32, p_ctx);
   sm3_update_mb8((const int8u **)pa_b,  len_32, p_ctx);
   sm3_update_mb8((const int8u **)pa_xG, len_32, p_ctx);
   sm3_update_mb8((const int8u **)pa_yG, len_32, p_ctx);

   sm3_update_mb8((const int8u **)pa_pubx, len_32, p_ctx);
   sm3_update_mb8((const int8u **)pa_puby, len_32, p_ctx);

   sm3_final_mb8(pa_z_digest, p_ctx);

}

static void sm2_ecdsa_compute_msg_digest(int8u* pa_msg_digest[8],
                                   const int8u* pa_z_digest[8],
                                   const int8u* pa_msg[8],
                                      const int msg_len[8])
{
   SM3_CTX_mb8 ctx;
   SM3_CTX_mb8* p_ctx = &ctx;

   sm3_init_mb8(p_ctx);

   sm3_update_mb8((const int8u **)pa_z_digest, len_32, p_ctx);
   sm3_update_mb8((const int8u **)pa_msg, (int*)msg_len, p_ctx);

   sm3_final_mb8(pa_msg_digest, p_ctx);
}

static void reverse_inplace(int8u* out, const int8u* inp, int len)
{
   for(int i=0; i<len/2; i++) {
      int8u a = inp[i];
      out[i] = inp[len-1-i];
      out[len-1-i] = a;
   }
}

/*
// SM2_POINT* P - Public key in Jacobian projective coordinates, coordinates are in Montgomery over p
// int64u* pa_rev_bytes_pubX[8] - X coordinate (affine), bytes reversed for future hashing
// int64u* pa_rev_bytes_pubY[8] - Y coordinate (affine), bytes reversed for future hashing
*/
static mbx_status sm2_ecdsa_process_pubkeys(SM2_POINT* P,
                                            int64u* pa_rev_bytes_pubX[8],
                                            int64u* pa_rev_bytes_pubY[8],
                                            int use_jproj_coords,
                                            mbx_status current_status)
{
   mbx_status status = current_status;

   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_psm2_)(P->X), MBX_STATUS_MISMATCH_PARAM_ERR);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_psm2_)(P->Y), MBX_STATUS_MISMATCH_PARAM_ERR);

   if(use_jproj_coords) {
      status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_psm2_)(P->Z), MBX_STATUS_MISMATCH_PARAM_ERR);

      MB_FUNC_NAME(ifma_tomont52_psm2_)(P->X, P->X);
      MB_FUNC_NAME(ifma_tomont52_psm2_)(P->Y, P->Y);
      MB_FUNC_NAME(ifma_tomont52_psm2_)(P->Z, P->Z);

      __ALIGN64 U64 X[PSM2_LEN52];
      __ALIGN64 U64 Y[PSM2_LEN52];

      MB_FUNC_NAME(get_sm2_ec_affine_coords_)(X, Y, P);

      /* convert P coordinates to regular domain */
      MB_FUNC_NAME(ifma_frommont52_psm2_)(X, X);
      MB_FUNC_NAME(ifma_frommont52_psm2_)(Y, Y);

      /* convert P coordinates to radix 64 */
      ifma_mb8_to_BNU(pa_rev_bytes_pubX, (const int64u (*)[8])X, PSM2_BITSIZE);
      ifma_mb8_to_BNU(pa_rev_bytes_pubY, (const int64u (*)[8])Y, PSM2_BITSIZE);
   } else {
      ifma_mb8_to_BNU(pa_rev_bytes_pubX, (const int64u (*)[8])P->X, PSM2_BITSIZE);
      ifma_mb8_to_BNU(pa_rev_bytes_pubY, (const int64u (*)[8])P->Y, PSM2_BITSIZE);

      MB_FUNC_NAME(ifma_tomont52_psm2_)(P->X, P->X);
      MB_FUNC_NAME(ifma_tomont52_psm2_)(P->Y, P->Y);
      MB_FUNC_NAME(ifma_tomont52_psm2_)(P->Z, (U64*)ones);
   }

   for(int i = 0; i < 8; i ++) {
      reverse_inplace((int8u*)pa_rev_bytes_pubX[i], (int8u*)pa_rev_bytes_pubX[i], 32);
      reverse_inplace((int8u*)pa_rev_bytes_pubY[i], (int8u*)pa_rev_bytes_pubY[i], 32);
   }

   return status;
}

/* compute and check signature components */
static mbx_status sm2_ecdsa_sign_mb8(U64 sign_r[],
                                     U64 sign_s[],
                                     U64 msg_digest[],
                                     U64 scalar_eph_skey[],
                                     U64 eph_skey[],
                                     U64 reg_skey[],
                                     mbx_status current_status)
{
   mbx_status status = current_status;

   /* compute public keys from ephemeral private keys */
   SM2_POINT P_eph;

   MB_FUNC_NAME(ifma_ec_sm2_mul_pointbase_)(&P_eph, scalar_eph_skey);

   /* extract affine P.x */
   MB_FUNC_NAME(ifma_aminv52_psm2_)(P_eph.Z, P_eph.Z);         /* 1/Z               */
   MB_FUNC_NAME(ifma_ams52_psm2_)(P_eph.Z, P_eph.Z);           /* 1/Z^2             */
   MB_FUNC_NAME(ifma_amm52_psm2_)(P_eph.X, P_eph.X, P_eph.Z);  /* x = (X) * (1/Z^2) */

   /* convert x-coordinate to regular domain */
   MB_FUNC_NAME(ifma_frommont52_psm2_)(P_eph.X, P_eph.X);

   /* compute r component */
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(sign_r, P_eph.X);
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(msg_digest, msg_digest);

   MB_FUNC_NAME(ifma_add52_nsm2_)(sign_r, sign_r, msg_digest);

   /* check r component */
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(eph_skey, eph_skey);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(sign_r), MBX_STATUS_SIGNATURE_ERR);

   __ALIGN64 U64 tmp[PSM2_LEN52];

   /* sign_r + eph_skey == n */
   MB_FUNC_NAME(ifma_add52_nsm2_)(tmp, sign_r, eph_skey);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(tmp), MBX_STATUS_SIGNATURE_ERR);

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   MB_FUNC_NAME(zero_)((int64u (*)[8])tmp, sizeof(tmp)/sizeof(U64));

   /* compute s component */
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(tmp, (U64*)ones);
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(reg_skey, reg_skey);

   MB_FUNC_NAME(ifma_amm52_nsm2_)(sign_s, sign_r, reg_skey);  /* r * d        */
   MB_FUNC_NAME(ifma_add52_nsm2_)(reg_skey, reg_skey, tmp);   /* 1 + d        */
   MB_FUNC_NAME(ifma_aminv52_nsm2_)(reg_skey, reg_skey);      /* (1 + d) ^ -1 */
   MB_FUNC_NAME(ifma_sub52_nsm2_)(sign_s, eph_skey, sign_s);  /* k - r * d    */
   MB_FUNC_NAME(ifma_amm52_nsm2_)(sign_s, sign_s, reg_skey);

   /* check s component */
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(sign_s), MBX_STATUS_SIGNATURE_ERR);

   return status;
}

/* verify signature components */
static mbx_status sm2_ecdsa_verify_mb8(U64 sign_r[],
                                            U64 sign_s[],
                                            U64 msg_digest[],
                                     SM2_POINT* P,
                                     mbx_status current_status)
{
   mbx_status status = current_status;

   __ALIGN64 U64 t[PSM2_LEN52];
   /* compute t = (r + s) mod n */
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(t, sign_r);
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(sign_s, sign_s);

   MB_FUNC_NAME(ifma_add52_nsm2_)(t, sign_s, t);

   /* check t != 0*/
   MB_FUNC_NAME(ifma_frommont52_nsm2_)(t, t);
   __mb_mask signature_err_mask = MB_FUNC_NAME(is_zero_FE256_)(t);

   int64u tmp[8][PSM2_LEN64];
   int64u* pa_tmp[8] = {tmp[0], tmp[1], tmp[2], tmp[3],
                        tmp[4], tmp[5], tmp[6], tmp[7]};

   /* convert sign_s to scalar */
   MB_FUNC_NAME(ifma_frommont52_nsm2_)(sign_s, sign_s);
   ifma_mb8_to_BNU(pa_tmp, (const int64u(*)[8])sign_s, PSM2_BITSIZE);
   ifma_BNU_transpose_copy((int64u (*)[8])sign_s, (const int64u(**))pa_tmp, PSM2_BITSIZE);
   sign_s[PSM2_LEN64] = get_zero64();

   SM2_POINT sG;
   /* compute [s]G */
   MB_FUNC_NAME(ifma_ec_sm2_mul_pointbase_)(&sG, sign_s);

   /* convert t to scalar */
   ifma_mb8_to_BNU(pa_tmp, (const int64u(*)[8])t, PSM2_BITSIZE);
   ifma_BNU_transpose_copy((int64u(*)[8])t, (const int64u(**))pa_tmp, PSM2_BITSIZE);
   t[PSM2_LEN64] = get_zero64();

   /* compute [s]G + [t]P */
   MB_FUNC_NAME(ifma_ec_sm2_mul_point_)(P, P, t);
   MB_FUNC_NAME(ifma_ec_sm2_add_point_)(P, P, &sG);

   __ALIGN64 U64 sign_r_restored[PSM2_LEN52];

   /* extract affine P.x */
   MB_FUNC_NAME(ifma_aminv52_psm2_)(P->Z, P->Z);                /* 1/Z               */
   MB_FUNC_NAME(ifma_ams52_psm2_)(P->Z, P->Z);                  /* 1/Z^2             */
   MB_FUNC_NAME(ifma_amm52_psm2_)(sign_r_restored, P->X, P->Z); /* x = (X) * (1/Z^2) */

   /* convert x-coordinate to Montgomery over n */
   MB_FUNC_NAME(ifma_frommont52_psm2_)(sign_r_restored, sign_r_restored);
   MB_FUNC_NAME(ifma_tomont52_nsm2_)(sign_r_restored, sign_r_restored);

   MB_FUNC_NAME(ifma_tomont52_nsm2_)(msg_digest, msg_digest);
   MB_FUNC_NAME(ifma_add52_nsm2_)(sign_r_restored, sign_r_restored, msg_digest);

   MB_FUNC_NAME(ifma_frommont52_nsm2_)(sign_r_restored, sign_r_restored);
   signature_err_mask |= ~(MB_FUNC_NAME(cmp_eq_FESM2_)(sign_r_restored, sign_r));

   status |= MBX_SET_STS_BY_MASK(status, signature_err_mask, MBX_STATUS_SIGNATURE_ERR);
   return status;
}

/*
// Computes SM2 ECDSA signature
// pa_sign_r[]       array of pointers to the computed r-components of the signatures
// pa_sign_s[]       array of pointers to the computed s-components of the signatures
// pa_user_id[]      array of pointers to the users ID
// user_id_len[]     array of users ID length
// pa_msg[]          array of pointers to the messages are being signed
// msg_len[]         array of messages length
// pa_eph_skey[]     array of pointers to the signer's ephemeral private keys
// pa_reg_skey[]     array of pointers to the signer's regular private keys
// pa_pubx[]         array of pointers to the party's public keys X-coordinates
// pa_puby[]         array of pointers to the party's public keys Y-coordinates
// pa_pubz[]         array of pointers to the party's public keys Z-coordinates
// pBuffer           pointer to the scratch buffer
*/
DLL_PUBLIC
mbx_status mbx_sm2_ecdsa_sign_mb8(int8u* pa_sign_r[8],
                                  int8u* pa_sign_s[8],
                      const int8u* const pa_user_id[8],
                               const int user_id_len[8],
                      const int8u* const pa_msg[8],
                               const int msg_len[8],
                     const int64u* const pa_eph_skey[8],
                     const int64u* const pa_reg_skey[8],
                     const int64u* const pa_pubx[8],
                     const int64u* const pa_puby[8],
                     const int64u* const pa_pubz[8],
                                  int8u* pBuffer)
{
   mbx_status status = 0;
   int use_jproj_coords = NULL!=pa_pubz;

   /* test input pointers */
   if(NULL==pa_sign_r || NULL==pa_sign_s   || NULL==pa_user_id  || NULL==user_id_len || NULL==pa_msg ||
      NULL==msg_len   || NULL==pa_eph_skey || NULL==pa_reg_skey || NULL==pa_pubx     || NULL==pa_puby) {
      status = MBX_SET_STS_ALL(MBX_STATUS_NULL_PARAM_ERR);
      return status;
   }

   int user_id_len_checked[8];

   /* check pointers and values */
   for(int buf_no=0; buf_no<8; buf_no++) {
      const int8u* r = pa_sign_r[buf_no];
      const int8u* s = pa_sign_s[buf_no];
      const int8u* id = pa_user_id[buf_no];
      const int8u* msg = pa_msg[buf_no];
      const int64u* eph_key = pa_eph_skey[buf_no];
      const int64u* reg_key = pa_reg_skey[buf_no];
      const int64u* pubx = pa_pubx[buf_no];
      const int64u* puby = pa_puby[buf_no];
      const int64u* pubz = use_jproj_coords? pa_pubz[buf_no] : NULL;

      user_id_len_checked[buf_no] = user_id_len[buf_no];

      /* if any of pointer NULL set error status */
      if(NULL==r       || NULL==s    || NULL==id   || NULL==msg || NULL==eph_key ||
         NULL==reg_key || NULL==pubx || NULL==puby || (use_jproj_coords && NULL==pubz)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_NULL_PARAM_ERR);
      }
      if (msg_len[buf_no] < 0) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
      }
      if ((user_id_len[buf_no] > 0xFFFF) || (user_id_len[buf_no] < 0)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
         user_id_len_checked[buf_no] = 0;
      }
   }

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   /* load and check secret keys */

   __ALIGN64 U64 reg_skey[PSM2_LEN52];
   __ALIGN64 U64 eph_skey[PSM2_LEN52];

   ifma_BNU_to_mb8((int64u (*)[8])reg_skey, pa_reg_skey, PSM2_BITSIZE);
   ifma_BNU_to_mb8((int64u (*)[8])eph_skey, pa_eph_skey, PSM2_BITSIZE);

   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(reg_skey), MBX_STATUS_MISMATCH_PARAM_ERR);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(eph_skey), MBX_STATUS_MISMATCH_PARAM_ERR);

   if(!MBX_IS_ANY_OK_STS(status)) {
      /* clear copy of the secret keys */
      MB_FUNC_NAME(zero_)((int64u (*)[8])reg_skey, sizeof(reg_skey)/sizeof(U64));
      MB_FUNC_NAME(zero_)((int64u (*)[8])eph_skey, sizeof(eph_skey)/sizeof(U64));
      return status;
   }

   __ALIGN64 int64u rev_bytes_pubX[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubX[8] = {rev_bytes_pubX[0], rev_bytes_pubX[1], rev_bytes_pubX[2], rev_bytes_pubX[3], rev_bytes_pubX[4], rev_bytes_pubX[5], rev_bytes_pubX[6], rev_bytes_pubX[7]};

   __ALIGN64 int64u rev_bytes_pubY[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubY[8] = {rev_bytes_pubY[0], rev_bytes_pubY[1], rev_bytes_pubY[2], rev_bytes_pubY[3], rev_bytes_pubY[4], rev_bytes_pubY[5], rev_bytes_pubY[6], rev_bytes_pubY[7]};

   SM2_POINT P;

   ifma_BNU_to_mb8((int64u (*)[8])P.X, (const int64u* (*))pa_pubx, PSM2_BITSIZE);
   ifma_BNU_to_mb8((int64u (*)[8])P.Y, (const int64u* (*))pa_puby, PSM2_BITSIZE);

   if(use_jproj_coords) {
      ifma_BNU_to_mb8((int64u (*)[8])P.Z, (const int64u* (*))pa_pubz, PSM2_BITSIZE);
   }

   status = sm2_ecdsa_process_pubkeys(&P,
                                      pa_rev_bytes_pubX,
                                      pa_rev_bytes_pubY,
                                      use_jproj_coords,
                                      status);

   if(!MBX_IS_ANY_OK_STS(status)) {
      /* clear copy of the secret keys */
      MB_FUNC_NAME(zero_)((int64u (*)[8])reg_skey, sizeof(reg_skey)/sizeof(U64));
      MB_FUNC_NAME(zero_)((int64u (*)[8])eph_skey, sizeof(eph_skey)/sizeof(U64));
      return status;
   }

   __ALIGN64 int8u msg_digest[8][PSM2_LEN8];
   int8u* pa_msg_digest[8] = {msg_digest[0], msg_digest[1], msg_digest[2], msg_digest[3], msg_digest[4], msg_digest[5], msg_digest[6], msg_digest[7]};

   /* compute z digest */
   sm2_ecdsa_compute_z_digest(pa_msg_digest,
              (const int8u **)pa_user_id,
                 (const int *)user_id_len_checked,
              (const int8u **)pa_rev_bytes_pubX,
              (const int8u **)pa_rev_bytes_pubY);

   /* compute msg digest */
   sm2_ecdsa_compute_msg_digest(pa_msg_digest,
                (const int8u **)pa_msg_digest,
                (const int8u **)pa_msg,
                   (const int *)msg_len);

   /* zero padded keys */
   U64 scalar_eph_skey[PSM2_LEN64+1];
   ifma_BNU_transpose_copy((int64u (*)[8])scalar_eph_skey, pa_eph_skey, PSM2_BITSIZE);
   scalar_eph_skey[PSM2_LEN64] = get_zero64();

   __ALIGN64 U64 sign_r[PSM2_LEN52];
   __ALIGN64 U64 sign_s[PSM2_LEN52];

   __ALIGN64 U64 msg[PSM2_LEN52];
   ifma_HexStr8_to_mb8((int64u (*)[8])msg, (const int8u* const*)pa_msg_digest, PSM2_BITSIZE);

   /* compute and check signature components */
   status = sm2_ecdsa_sign_mb8(sign_r,
                               sign_s,
                               msg,
                               scalar_eph_skey,
                               eph_skey,
                               reg_skey,
                               status);

   /* clear copy of the secret keys */
   MB_FUNC_NAME(zero_)((int64u (*)[8])scalar_eph_skey, sizeof(scalar_eph_skey)/sizeof(U64));
   MB_FUNC_NAME(zero_)((int64u (*)[8])reg_skey, sizeof(reg_skey)/sizeof(U64));
   MB_FUNC_NAME(zero_)((int64u (*)[8])eph_skey, sizeof(eph_skey)/sizeof(U64));

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   MB_FUNC_NAME(ifma_frommont52_nsm2_)(sign_r, sign_r);
   ifma_mb8_to_HexStr8(pa_sign_r, (const int64u(*)[8])sign_r, PSM2_BITSIZE);

   MB_FUNC_NAME(ifma_frommont52_nsm2_)(sign_s, sign_s);
   ifma_mb8_to_HexStr8(pa_sign_s, (const int64u(*)[8])sign_s, PSM2_BITSIZE);

   return status;
}

/*
// Verifies SM2 ECDSA signature
// pa_sign_r[]       array of pointers to the computed r-components of the signatures
// pa_sign_s[]       array of pointers to the computed s-components of the signatures
// pa_msg[]          array of pointers to the messages that have been signed
// pa_user_id[]      array of pointers to the users ID
// user_id_len[]     array of users ID length
// pa_msg[]          array of pointers to the messages are being signed
// msg_len[]         array of messages length
// pa_pubx[]         array of pointers to the signer's public keys X-coordinates
// pa_puby[]         array of pointers to the signer's public keys Y-coordinates
// pa_pubz[]         array of pointers to the signer's public keys Z-coordinates  (or NULL, if affine coordinate requested)
// pBuffer           pointer to the scratch buffer
*/
DLL_PUBLIC
mbx_status mbx_sm2_ecdsa_verify_mb8(const int8u* const pa_sign_r[8],
                                    const int8u* const pa_sign_s[8],
                                    const int8u* const pa_user_id[8],
                                             const int user_id_len[8],
                                    const int8u* const pa_msg[8],
                                             const int msg_len[8],
                                   const int64u* const pa_pubx[8],
                                   const int64u* const pa_puby[8],
                                   const int64u* const pa_pubz[8],
                                                int8u* pBuffer)
{
   mbx_status status = 0;
   int use_jproj_coords = NULL!=pa_pubz;

   /* test input pointers */
   if(NULL==pa_sign_r || NULL==pa_sign_s || NULL==pa_user_id  || NULL==user_id_len ||
      NULL==pa_msg    || NULL==msg_len   || NULL==pa_pubx     || NULL==pa_puby) {
      status = MBX_SET_STS_ALL(MBX_STATUS_NULL_PARAM_ERR);
      return status;
   }

   int user_id_len_checked[8];

   /* check pointers and values */
   for(int buf_no=0; buf_no<8; buf_no++) {
      const int8u* r = pa_sign_r[buf_no];
      const int8u* s = pa_sign_s[buf_no];
      const int8u* id = pa_user_id[buf_no];
      const int8u* msg = pa_msg[buf_no];
      const int64u* pubx = pa_pubx[buf_no];
      const int64u* puby = pa_puby[buf_no];
      const int64u* pubz = use_jproj_coords? pa_pubz[buf_no] : NULL;

      user_id_len_checked[buf_no] = user_id_len[buf_no];

      /* if any of pointer NULL set error status */
      if(NULL==r    || NULL==s    || NULL==id   || NULL==msg ||
         NULL==pubx || NULL==puby || (use_jproj_coords && NULL==pubz)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_NULL_PARAM_ERR);
      }
      if (msg_len[buf_no] < 0) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
      }
      if ((user_id_len[buf_no] > 0xFFFF) || (user_id_len[buf_no] < 0)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
         user_id_len_checked[buf_no] = 0;
      }
   }

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   __ALIGN64 U64 sign_r[PSM2_LEN52];
   __ALIGN64 U64 sign_s[PSM2_LEN52];

   ifma_HexStr8_to_mb8((int64u (*)[8])sign_r, (const int8u* const*)pa_sign_r, PSM2_BITSIZE);
   ifma_HexStr8_to_mb8((int64u (*)[8])sign_s, (const int8u* const*)pa_sign_s, PSM2_BITSIZE);

   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_nsm2_)(sign_r), MBX_STATUS_MISMATCH_PARAM_ERR);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_nsm2_)(sign_s), MBX_STATUS_MISMATCH_PARAM_ERR);

   if(!MBX_IS_ANY_OK_STS(status))
      return status;

   __ALIGN64 int64u rev_bytes_pubX[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubX[8] = {rev_bytes_pubX[0], rev_bytes_pubX[1], rev_bytes_pubX[2], rev_bytes_pubX[3], rev_bytes_pubX[4], rev_bytes_pubX[5], rev_bytes_pubX[6], rev_bytes_pubX[7]};

   __ALIGN64 int64u rev_bytes_pubY[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubY[8] = {rev_bytes_pubY[0], rev_bytes_pubY[1], rev_bytes_pubY[2], rev_bytes_pubY[3], rev_bytes_pubY[4], rev_bytes_pubY[5], rev_bytes_pubY[6], rev_bytes_pubY[7]};

   SM2_POINT P;

   ifma_BNU_to_mb8((int64u (*)[8])P.X, (const int64u* (*))pa_pubx, PSM2_BITSIZE);
   ifma_BNU_to_mb8((int64u (*)[8])P.Y, (const int64u* (*))pa_puby, PSM2_BITSIZE);

   if(use_jproj_coords) {
      ifma_BNU_to_mb8((int64u (*)[8])P.Z, (const int64u* (*))pa_pubz, PSM2_BITSIZE);
   }

   status = sm2_ecdsa_process_pubkeys(&P,
                                      pa_rev_bytes_pubX,
                                      pa_rev_bytes_pubY,
                                      use_jproj_coords,
                                      status);

   if(!MBX_IS_ANY_OK_STS(status))
      return status;

   __ALIGN64 int8u msg_digest[8][PSM2_LEN8];
   int8u* pa_msg_digest[8] = {msg_digest[0], msg_digest[1], msg_digest[2], msg_digest[3], msg_digest[4], msg_digest[5], msg_digest[6], msg_digest[7]};

   /* compute z digest */
   sm2_ecdsa_compute_z_digest(pa_msg_digest,
              (const int8u **)pa_user_id,
                 (const int *)user_id_len_checked,
              (const int8u **)pa_rev_bytes_pubX,
              (const int8u **)pa_rev_bytes_pubY);

   /* compute msg digest */
   sm2_ecdsa_compute_msg_digest(pa_msg_digest,
                (const int8u **)pa_msg_digest,
                (const int8u **)pa_msg,
                   (const int *)msg_len);

   __ALIGN64 U64 msg[PSM2_LEN52];
   ifma_HexStr8_to_mb8((int64u (*)[8])msg, (const int8u* const*)pa_msg_digest, PSM2_BITSIZE);

   status = sm2_ecdsa_verify_mb8(sign_r, sign_s, msg, &P, status);

   return status;
}

/*
// OpenSSL's specific implementations
*/
#ifndef BN_OPENSSL_DISABLE

DLL_PUBLIC
mbx_status mbx_sm2_ecdsa_sign_ssl_mb8(int8u* pa_sign_r[8],
                                      int8u* pa_sign_s[8],
                          const int8u* const pa_user_id[8],
                                   const int user_id_len[8],
                          const int8u* const pa_msg[8],
                                   const int msg_len[8],
                         const BIGNUM* const pa_eph_skey[8],
                         const BIGNUM* const pa_reg_skey[8],
                         const BIGNUM* const pa_pubx[8],
                         const BIGNUM* const pa_puby[8],
                         const BIGNUM* const pa_pubz[8],
                                      int8u* pBuffer)
{
   mbx_status status = 0;
   int use_jproj_coords = NULL!=pa_pubz;

   /* test input pointers */
   if(NULL==pa_sign_r || NULL==pa_sign_s   || NULL==pa_user_id  || NULL==user_id_len || NULL==pa_msg ||
      NULL==msg_len   || NULL==pa_eph_skey || NULL==pa_reg_skey || NULL==pa_pubx     || NULL==pa_puby) {
      status = MBX_SET_STS_ALL(MBX_STATUS_NULL_PARAM_ERR);
      return status;
   }

   int user_id_len_checked[8];

   /* check pointers and values */
   for(int buf_no=0; buf_no<8; buf_no++) {
      const int8u* r = pa_sign_r[buf_no];
      const int8u* s = pa_sign_s[buf_no];
      const int8u* id = pa_user_id[buf_no];
      const int8u* msg = pa_msg[buf_no];
      const BIGNUM* eph_key = pa_eph_skey[buf_no];
      const BIGNUM* reg_key = pa_reg_skey[buf_no];
      const BIGNUM* pubx = pa_pubx[buf_no];
      const BIGNUM* puby = pa_puby[buf_no];
      const BIGNUM* pubz = use_jproj_coords? pa_pubz[buf_no] : NULL;

      user_id_len_checked[buf_no] = user_id_len[buf_no];

      /* if any of pointer NULL set error status */
      if(NULL==r       || NULL==s    || NULL==id   || NULL==msg || NULL==eph_key ||
         NULL==reg_key || NULL==pubx || NULL==puby || (use_jproj_coords && NULL==pubz)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_NULL_PARAM_ERR);
      }
      if (msg_len[buf_no] < 0) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
      }
      if ((user_id_len[buf_no] > 0xFFFF) || (user_id_len[buf_no] < 0)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
         user_id_len_checked[buf_no] = 0;
      }
   }

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   /* load and check secret keys */

   __ALIGN64 U64 reg_skey[PSM2_LEN52];
   __ALIGN64 U64 eph_skey[PSM2_LEN52];

   ifma_BN_to_mb8((int64u (*)[8])reg_skey, pa_reg_skey, PSM2_BITSIZE);
   ifma_BN_to_mb8((int64u (*)[8])eph_skey, pa_eph_skey, PSM2_BITSIZE);

   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(reg_skey), MBX_STATUS_MISMATCH_PARAM_ERR);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(is_zero_FESM2_)(eph_skey), MBX_STATUS_MISMATCH_PARAM_ERR);

   if(!MBX_IS_ANY_OK_STS(status)) {
      /* clear copy of the secret keys */
      MB_FUNC_NAME(zero_)((int64u (*)[8])reg_skey, sizeof(reg_skey)/sizeof(U64));
      MB_FUNC_NAME(zero_)((int64u (*)[8])eph_skey, sizeof(eph_skey)/sizeof(U64));
      return status;
   }

   __ALIGN64 int64u rev_bytes_pubX[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubX[8] = {rev_bytes_pubX[0], rev_bytes_pubX[1], rev_bytes_pubX[2], rev_bytes_pubX[3], rev_bytes_pubX[4], rev_bytes_pubX[5], rev_bytes_pubX[6], rev_bytes_pubX[7]};

   __ALIGN64 int64u rev_bytes_pubY[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubY[8] = {rev_bytes_pubY[0], rev_bytes_pubY[1], rev_bytes_pubY[2], rev_bytes_pubY[3], rev_bytes_pubY[4], rev_bytes_pubY[5], rev_bytes_pubY[6], rev_bytes_pubY[7]};

   SM2_POINT P;

   ifma_BN_to_mb8((int64u (*)[8])P.X, pa_pubx, PSM2_BITSIZE);
   ifma_BN_to_mb8((int64u (*)[8])P.Y, pa_puby, PSM2_BITSIZE);

   if(use_jproj_coords) {
      ifma_BN_to_mb8((int64u (*)[8])P.Z, pa_pubz, PSM2_BITSIZE);
   }

   status = sm2_ecdsa_process_pubkeys(&P,
                                      pa_rev_bytes_pubX,
                                      pa_rev_bytes_pubY,
                                      use_jproj_coords,
                                      status);

   if(!MBX_IS_ANY_OK_STS(status)) {
      /* clear copy of the secret keys */
      MB_FUNC_NAME(zero_)((int64u (*)[8])reg_skey, sizeof(reg_skey)/sizeof(U64));
      MB_FUNC_NAME(zero_)((int64u (*)[8])eph_skey, sizeof(eph_skey)/sizeof(U64));
      return status;
   }

   __ALIGN64 int8u msg_digest[8][PSM2_LEN8];
   int8u* pa_msg_digest[8] = {msg_digest[0], msg_digest[1], msg_digest[2], msg_digest[3], msg_digest[4], msg_digest[5], msg_digest[6], msg_digest[7]};

   /* compute z digest */
   sm2_ecdsa_compute_z_digest(pa_msg_digest,
              (const int8u **)pa_user_id,
                 (const int *)user_id_len_checked,
              (const int8u **)pa_rev_bytes_pubX,
              (const int8u **)pa_rev_bytes_pubY);

   /* compute msg digest */
   sm2_ecdsa_compute_msg_digest(pa_msg_digest,
                (const int8u **)pa_msg_digest,
                (const int8u **)pa_msg,
                   (const int *)msg_len);

   /* zero padded keys */
   U64 scalar_eph_skey[PSM2_LEN64+1];
   ifma_BN_transpose_copy((int64u (*)[8])scalar_eph_skey, pa_eph_skey, PSM2_BITSIZE);
   scalar_eph_skey[PSM2_LEN64] = get_zero64();

   __ALIGN64 U64 sign_r[PSM2_LEN52];
   __ALIGN64 U64 sign_s[PSM2_LEN52];

   __ALIGN64 U64 msg[PSM2_LEN52];
   ifma_HexStr8_to_mb8((int64u (*)[8])msg, (const int8u* const*)pa_msg_digest, PSM2_BITSIZE);

   /* compute and check signature components */
   status = sm2_ecdsa_sign_mb8(sign_r,
                               sign_s,
                               msg,
                               scalar_eph_skey,
                               eph_skey,
                               reg_skey,
                               status);

   /* clear copy of the secret keys */
   MB_FUNC_NAME(zero_)((int64u (*)[8])scalar_eph_skey, sizeof(scalar_eph_skey)/sizeof(U64));
   MB_FUNC_NAME(zero_)((int64u (*)[8])reg_skey, sizeof(reg_skey)/sizeof(U64));
   MB_FUNC_NAME(zero_)((int64u (*)[8])eph_skey, sizeof(eph_skey)/sizeof(U64));

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   MB_FUNC_NAME(ifma_frommont52_nsm2_)(sign_r, sign_r);
   ifma_mb8_to_HexStr8(pa_sign_r, (const int64u(*)[8])sign_r, PSM2_BITSIZE);

   MB_FUNC_NAME(ifma_frommont52_nsm2_)(sign_s, sign_s);
   ifma_mb8_to_HexStr8(pa_sign_s, (const int64u(*)[8])sign_s, PSM2_BITSIZE);

   return status;
}

DLL_PUBLIC
mbx_status mbx_sm2_ecdsa_verify_ssl_mb8(const ECDSA_SIG* const pa_sig[8],
                                            const int8u* const pa_user_id[8],
                                                     const int user_id_len[8],
                                            const int8u* const pa_msg[8],
                                                     const int msg_len[8],
                                           const BIGNUM* const pa_pubx[8],
                                           const BIGNUM* const pa_puby[8],
                                           const BIGNUM* const pa_pubz[8],
                                                        int8u* pBuffer)
{
   mbx_status status = 0;
   int use_jproj_coords = NULL!=pa_pubz;

   /* test input pointers */
   if(NULL==pa_sig  || NULL==pa_user_id  || NULL==user_id_len || NULL==pa_msg ||
      NULL==msg_len || NULL==pa_pubx     || NULL==pa_puby) {
      status = MBX_SET_STS_ALL(MBX_STATUS_NULL_PARAM_ERR);
      return status;
   }

   int user_id_len_checked[8];

   /* check pointers and values */
   for(int buf_no=0; buf_no<8; buf_no++) {
      const ECDSA_SIG* sig = pa_sig[buf_no];
      const int8u* id = pa_user_id[buf_no];
      const int8u* msg = pa_msg[buf_no];
      const BIGNUM* pubx = pa_pubx[buf_no];
      const BIGNUM* puby = pa_puby[buf_no];
      const BIGNUM* pubz = use_jproj_coords? pa_pubz[buf_no] : NULL;

      user_id_len_checked[buf_no] = user_id_len[buf_no];

      /* if any of pointer NULL set error status */
      if(NULL==sig  || NULL==id   || NULL==msg ||
         NULL==pubx || NULL==puby || (use_jproj_coords && NULL==pubz)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_NULL_PARAM_ERR);
      }
      if (msg_len[buf_no] < 0) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
      }
      if ((user_id_len[buf_no] > 0xFFFF) || (user_id_len[buf_no] < 0)) {
         status |= MBX_SET_STS(status, buf_no, MBX_STATUS_MISMATCH_PARAM_ERR);
         user_id_len_checked[buf_no] = 0;
      }
   }

   if(!MBX_IS_ANY_OK_STS(status) )
      return status;

   BIGNUM* pa_sign_r[8] = { 0,0,0,0,0,0,0,0 };
   BIGNUM* pa_sign_s[8] = { 0,0,0,0,0,0,0,0 };

   for (int buf_no = 0; buf_no < 8; buf_no++)
   {
      if(pa_sig[buf_no] != NULL)
      {
         ECDSA_SIG_get0(pa_sig[buf_no], (const BIGNUM(**))pa_sign_r + buf_no,
                                        (const BIGNUM(**))pa_sign_s + buf_no);
      }
   }

   __ALIGN64 U64 sign_r[PSM2_LEN52];
   __ALIGN64 U64 sign_s[PSM2_LEN52];

   ifma_BN_to_mb8((int64u (*)[8])sign_r, (const BIGNUM(**))pa_sign_r, PSM2_BITSIZE);
   ifma_BN_to_mb8((int64u (*)[8])sign_s, (const BIGNUM(**))pa_sign_s, PSM2_BITSIZE);

   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_nsm2_)(sign_r), MBX_STATUS_MISMATCH_PARAM_ERR);
   status |= MBX_SET_STS_BY_MASK(status, MB_FUNC_NAME(ifma_check_range_nsm2_)(sign_s), MBX_STATUS_MISMATCH_PARAM_ERR);

   if(!MBX_IS_ANY_OK_STS(status))
      return status;

   __ALIGN64 int64u rev_bytes_pubX[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubX[8] = {rev_bytes_pubX[0], rev_bytes_pubX[1], rev_bytes_pubX[2], rev_bytes_pubX[3], rev_bytes_pubX[4], rev_bytes_pubX[5], rev_bytes_pubX[6], rev_bytes_pubX[7]};

   __ALIGN64 int64u rev_bytes_pubY[8][PSM2_LEN64];
   int64u* pa_rev_bytes_pubY[8] = {rev_bytes_pubY[0], rev_bytes_pubY[1], rev_bytes_pubY[2], rev_bytes_pubY[3], rev_bytes_pubY[4], rev_bytes_pubY[5], rev_bytes_pubY[6], rev_bytes_pubY[7]};

   SM2_POINT P;

   ifma_BN_to_mb8((int64u (*)[8])P.X, pa_pubx, PSM2_BITSIZE);
   ifma_BN_to_mb8((int64u (*)[8])P.Y, pa_puby, PSM2_BITSIZE);

   if(use_jproj_coords) {
      ifma_BN_to_mb8((int64u (*)[8])P.Z, pa_pubz, PSM2_BITSIZE);
   }

   status = sm2_ecdsa_process_pubkeys(&P,
                                      pa_rev_bytes_pubX,
                                      pa_rev_bytes_pubY,
                                      use_jproj_coords,
                                      status);

   if(!MBX_IS_ANY_OK_STS(status))
      return status;

   __ALIGN64 int8u msg_digest[8][PSM2_LEN8];
   int8u* pa_msg_digest[8] = {msg_digest[0], msg_digest[1], msg_digest[2], msg_digest[3], msg_digest[4], msg_digest[5], msg_digest[6], msg_digest[7]};

   /* compute z digest */
   sm2_ecdsa_compute_z_digest(pa_msg_digest,
              (const int8u **)pa_user_id,
                 (const int *)user_id_len_checked,
              (const int8u **)pa_rev_bytes_pubX,
              (const int8u **)pa_rev_bytes_pubY);

   /* compute msg digest */
   sm2_ecdsa_compute_msg_digest(pa_msg_digest,
                (const int8u **)pa_msg_digest,
                (const int8u **)pa_msg,
                   (const int *)msg_len);

   __ALIGN64 U64 msg[PSM2_LEN52];
   ifma_HexStr8_to_mb8((int64u (*)[8])msg, (const int8u* const*)pa_msg_digest, PSM2_BITSIZE);

   status = sm2_ecdsa_verify_mb8(sign_r, sign_s, msg, &P, status);

   return status;
}

#endif
