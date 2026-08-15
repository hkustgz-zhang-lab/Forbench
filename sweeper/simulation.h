#ifndef SIMULATION_H
#define SIMULATION_H


#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <gmp.h>
#include <iostream>
#include "gmpxx.h"
using namespace std;

//----------------CONFIG--------------
//------------------------------------
#define BTOR_USE_GMP 1



using BTOR_BV_TYPE = uint32_t;
constexpr uint32_t BTOR_BV_TYPE_BW = sizeof(BTOR_BV_TYPE)*8;
constexpr std::array<uint32_t, 3> hash_primes = {333444569u, 76891121u, 456790003u};
constexpr uint32_t NPRIMES = hash_primes.size();


constexpr uint32_t mask_rem_bits(uint32_t width) {
  return (((BTOR_BV_TYPE)1 << (BTOR_BV_TYPE_BW - 1)) - 1) >> (BTOR_BV_TYPE_BW - 1 - (width % BTOR_BV_TYPE_BW));
}

struct BtorBitVector {
  uint32_t width;
#ifdef BTOR_USE_GMP
  mpz_t val;
  BtorBitVector(uint32_t bw) : width(bw) {
      mpz_init(val);
  }
  ~BtorBitVector() {
      mpz_clear(val);
  }
#else
  uint32_t len;
  std::unique_ptr<BTOR_BV_TYPE[]> bits;

  BtorBitVector(uint32_t bw) : width(bw) {
      assert(bw > 0);
      len = bw / BTOR_BV_TYPE_BW + (bw % BTOR_BV_TYPE_BW > 0 ? 1 : 0);
      bits = std::make_unique<BTOR_BV_TYPE[]>(len);
      for (uint32_t i = 0; i < len; ++i) bits[i] = 0;
  }
#endif
};

class BtorMemMgr {
public:
  size_t allocated = 0;
  size_t maxallocated = 0;
  size_t sat_allocated = 0;
  size_t sat_maxallocated = 0;
};

using BtorBitVectorPtr = std::unique_ptr<BtorBitVector>;



inline BtorBitVectorPtr btor_bv_new(uint32_t bw) {
  return std::make_unique<BtorBitVector>(bw);
}

//TODO
inline uint32_t btor_bv_hash (const BtorBitVector *bv)
{
  assert (bv);

  uint32_t i, j = 0, n, res = 0;
  uint32_t x, p0, p1;

  res = bv->width * hash_primes[j++];

#ifdef BTOR_USE_GMP
  // least significant limb is at index 0
  mp_limb_t limb;
  for (i = 0, j = 0, n = mpz_size (bv->val); i < n; ++i)
  {
    p0 = hash_primes[j++];
    if (j == NPRIMES) j = 0;
    p1 = hash_primes[j++];
    if (j == NPRIMES) j = 0;
    limb = mpz_getlimbn (bv->val, i);
    if (mp_bits_per_limb == 64)
    {
      uint32_t lo = (uint32_t) limb;
      uint32_t hi = (uint32_t) (limb >> 32);
      x           = lo ^ res;
      x           = ((x >> 16) ^ x) * p0;
      x           = ((x >> 16) ^ x) * p1;
      x           = ((x >> 16) ^ x);
      p0          = hash_primes[j++];
      if (j == NPRIMES) j = 0;
      p1 = hash_primes[j++];
      if (j == NPRIMES) j = 0;
      x = x ^ hi;
    }
    else
    {
      assert (mp_bits_per_limb == 32);
      x = res ^ limb;
    }
    x   = ((x >> 16) ^ x) * p0;
    x   = ((x >> 16) ^ x) * p1;
    res = ((x >> 16) ^ x);
  }
#else
  for (i = 0, j = 0, n = bv->len; i < n; i++)
  {
    p0 = hash_primes[j++];
    if (j == NPRIMES) j = 0;
    p1 = hash_primes[j++];
    if (j == NPRIMES) j = 0;
    x   = bv->bits[i] ^ res;
    x   = ((x >> 16) ^ x) * p0;
    x   = ((x >> 16) ^ x) * p1;
    res = ((x >> 16) ^ x);
  }
#endif
  return res;
}


inline void btor_bv_set_bit(BtorBitVector& bv, uint32_t pos, uint32_t bit)
{
    assert(bit == 0 || bit == 1);
    assert(pos < bv.width);

#ifdef BTOR_USE_GMP
    if (bit)
    {
        mpz_setbit(bv.val, pos);
    }
    else
    {
        mpz_clrbit(bv.val, pos);
    }
#else
    assert(bv.len > 0);

    uint32_t i = pos / BTOR_BV_TYPE_BW;
    uint32_t j = pos % BTOR_BV_TYPE_BW;
    assert(i < bv.len);

    if (bit)
    {
        bv.bits[bv.len - 1 - i] |= (1u << j);
    }
    else
    {
        bv.bits[bv.len - 1 - i] &= ~(1u << j);
    }
#endif
}




inline BtorBitVectorPtr btor_bv_const(const char* str, uint32_t bw) 
{
    assert(strlen(str) <= bw);

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init_set_str(res->val, str, 2);
    return res;
#else
    auto res = btor_bv_new(bw);
    for (uint32_t i = 0; i < bw; i++)
    {
        uint32_t j = bw - 1 - i;
        uint32_t bit = (str[j] == '1') ? 1 : 0;
        btor_bv_set_bit(res.get(), i, bit);
    }
    return res;
#endif
}


inline BtorBitVectorPtr btor_bv_char_to_bv(const char* assignment)
{
    assert(assignment);
    size_t len = strlen(assignment);
    assert(len > 0);

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(len);
    res->width = static_cast<uint32_t>(len);
    mpz_init_set_str(res->val, assignment, 2);
    return res;
#else
    return btor_bv_const(assignment, static_cast<uint32_t>(len)); // already returns unique_ptr
#endif
}




inline uint32_t btor_bv_get_bit(const BtorBitVector& bv, uint32_t pos)
{
    assert(pos < bv.width);

#ifdef BTOR_USE_GMP
    return mpz_tstbit(bv.val, pos);
#else
    uint32_t i = pos / BTOR_BV_TYPE_BW;
    uint32_t j = pos % BTOR_BV_TYPE_BW;
    return (bv.bits[bv.len - 1 - i] >> j) & 1;
#endif
}




inline std::string btor_bv_to_string(const BtorBitVector& bv)
{
    uint64_t bw = bv.width;

#ifdef BTOR_USE_GMP
    std::string res(bw, '0');
    char* tmp = mpz_get_str(nullptr, 2, bv.val);
    uint64_t n = strlen(tmp);
    uint64_t diff = bw - n;

    assert(n <= bw);
    std::copy(tmp, tmp + n, res.begin() + diff);
    free(tmp);

    return res;
#else
    std::string res(bw, '0');
    for (uint64_t i = 0; i < bw; ++i)
    {
        uint32_t bit = btor_bv_get_bit(bv, i);
        res[bw - 1 - i] = bit ? '1' : '0';
    }
    return res;
#endif
}



inline uint64_t btor_bv_to_uint64 (const BtorBitVector *bv)
{
  assert (bv);
  assert (bv->width <= sizeof (uint64_t) * 8);

  uint64_t res;

#ifdef BTOR_USE_GMP
  res = mpz_get_ui (bv->val);
#else
  assert (bv->len <= 2);
  uint32_t i;
  res = 0;
  for (i = 0; i < bv->len; i++)
    res |= ((uint64_t) bv->bits[i]) << (BTOR_BV_TYPE_BW * (bv->len - 1 - i));
#endif

  return res;
}

//TODO:
#ifndef BTOR_USE_GMP
#ifndef NDEBUG
inline static bool rem_bits_zero_dbg (BtorBitVector *bv)
{
  return (bv->width % BTOR_BV_TYPE_BW == 0
          || (bv->bits[0] >> (bv->width % BTOR_BV_TYPE_BW) == 0));
}
#endif

inline static void set_rem_bits_to_zero (BtorBitVector *bv)
{
  if (bv->width != BTOR_BV_TYPE_BW * bv->len)
    bv->bits[0] &= BTOR_MASK_REM_BITS (bv);
}
#endif


inline BtorBitVectorPtr btor_bv_uint64_to_bv(uint64_t value, uint32_t bw)
{
    assert(bw > 0);

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init_set_ui(res->val, value);
    mpz_fdiv_r_2exp(res->val, res->val, bw); // truncate extra bits
    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    auto& bv = *(wrapper->bv);

    assert(bv.len > 0);
    if (bv.len >= 1)
        bv.bits[bv.len - 1] = static_cast<BTOR_BV_TYPE>(value);

    if (bv.width > BTOR_BV_TYPE_BW && bv.len >= 2)
        bv.bits[bv.len - 2] = static_cast<BTOR_BV_TYPE>(value >> BTOR_BV_TYPE_BW);

    set_rem_bits_to_zero(&bv);
    assert(rem_bits_zero_dbg(&bv));

    return std::move(wrapper->bv_ptr);
#endif
}



inline BtorBitVectorPtr btor_bv_add(const BtorBitVector& a, const BtorBitVector& b)
{
  if (a.width != b.width) {
    std::cerr << "[Error] btor_bv_add width mismatch: "
              << "a.width = " << a.width << ", "
              << "b.width = " << b.width << std::endl;
    assert(a.width == b.width);
}

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(a.width);
    res->width = a.width;
    mpz_init(res->val);
    mpz_add(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, a.width);
    return res;
#else
    if (a.width <= 64)
    {
        uint64_t x = btor_bv_to_uint64(&a);
        uint64_t y = btor_bv_to_uint64(&b);
        return btor_bv_from_uint64(x + y, a.width);
    }
    else
    {
        auto wrapper = std::make_unique<BtorBitVectorWrapper>(a.width);
        BtorBitVector* res = wrapper->bv;

        assert(a.len == b.len);
        BTOR_BV_TYPE carry = 0;

        for (int64_t i = a.len - 1; i >= 0; --i)
        {
            uint64_t sum = static_cast<uint64_t>(a.bits[i]) + b.bits[i] + carry;
            res->bits[i] = static_cast<BTOR_BV_TYPE>(sum);
            carry = static_cast<BTOR_BV_TYPE>(sum >> BTOR_BV_TYPE_BW);
        }

        set_rem_bits_to_zero(res);
        assert(rem_bits_zero_dbg(res));

        return std::move(wrapper->bv_ptr);
    }
#endif
}

inline BtorBitVectorPtr btor_bv_and(const BtorBitVector& a, const BtorBitVector& b)
{
    if (a.width != b.width) {
        std::cerr << "[Error] btor_bv_and width mismatch: "
                  << "a.width = " << a.width << ", "
                  << "b.width = " << b.width << std::endl;
        assert(a.width == b.width);
    }
    

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(a.width);
    res->width = a.width;
    mpz_init(res->val);
    mpz_and(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, a.width);
    return res;
#else
    assert(a.len == b.len);
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(a.width);
    BtorBitVector* res = wrapper->bv;

    for (uint32_t i = 0; i < a.len; ++i)
        res->bits[i] = a.bits[i] & b.bits[i];

    assert(rem_bits_zero_dbg(res));
    return std::move(wrapper->bv_ptr);
#endif
}


inline bool btor_bv_is_zero (const BtorBitVector &bv)
{

#ifdef BTOR_USE_GMP
  return mpz_cmp_ui (bv.val, 0) == 0;
#else
  uint32_t i;
  for (i = 0; i < bv.len; i++)
    if (bv.bits[i] != 0) return false;
  return true;
#endif
}


inline bool btor_bv_is_one(const BtorBitVector& bv)
{
#ifdef BTOR_USE_GMP
    return mpz_cmp_ui(bv.val, 1) == 0;
#else
    if (bv.bits[bv.len - 1] != 1) return false;
    for (uint32_t i = 0; i < bv.len - 1; ++i)
    {
        if (bv.bits[i] != 0) return false;
    }
    return true;
#endif
}


inline BtorBitVectorPtr btor_bv_one(uint32_t bw)
{
    assert(bw > 0);

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init_set_ui(res->val, 1);
    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv;

    btor_bv_set_bit(res, 0, 1);

    return std::move(wrapper->bv_ptr);
#endif
}

//TODO
#define btor_bv_zero(BW) btor_bv_new (BW)

inline BtorBitVectorPtr btor_bv_implies(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);
    assert(a.width == 1);

#ifdef BTOR_USE_GMP
    if (btor_bv_is_zero(a) || btor_bv_is_one(b)) {
        return btor_bv_one(1);
    } else {
        return btor_bv_zero(1);
    }
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(a.width);
    BtorBitVector* res = wrapper->bv.get();

    for (uint32_t i = 0; i < a.len; ++i)
        res->bits[i] = ~a.bits[i] | b.bits[i];

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_or(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init(res->val);
    mpz_ior(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);
    return res;
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    auto& res = wrapper->bv;

    for (uint32_t i = 0; i < a->len; ++i)
        res->bits[i] = a.bits[i] | b.bits[i];

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));

    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_nand(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init(res->val);

    mpz_and(res->val, a.val, b.val);
    mpz_com(res->val, res->val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);

    return res;
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    auto& res = wrapper->bv;

    for (uint32_t i = 0; i < a.len; ++i)
        res->bits[i] = ~(a.bits[i] & b.bits[i]);

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));

    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_nor(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init(res->val);

    mpz_ior(res->val, a.val, b.val);
    mpz_com(res->val, res->val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);

    return res;
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    auto& res = wrapper->bv;

    for (uint32_t i = 0; i < a.len; ++i)
        res->bits[i] = ~(a.bits[i] | b.bits[i]);

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));

    return std::move(wrapper->bv_ptr);
#endif
}




inline BtorBitVectorPtr btor_bv_not(const BtorBitVector& bv)
{
    const uint32_t bw = bv.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init(res->val);

    mpz_com(res->val, bv.val);               // res = ~bv
    mpz_fdiv_r_2exp(res->val, res->val, bw); // truncate to width

    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv.get();

    for (uint32_t i = 0; i < bv.len; ++i)
        res->bits[i] = ~bv.bits[i];

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}


//TODO
#if 0
#define BTOR_LOG_MEM(FMT, ARGS...)   \
  do                                 \
  {                                  \
    fputs ("[btorlogmem] ", stdout); \
    printf (FMT, ##ARGS);            \
    fflush (stdout);                 \
  } while (0)
#else
#define BTOR_LOG_MEM(...) \
  do                      \
  {                       \
  } while (0)
#endif

inline void btor_mem_free (void *p, size_t freed)
{
  assert (!p == !freed);
  BTOR_LOG_MEM ("%p free   %10ld\n", p, freed);
  free (p);
}



inline int32_t btor_bv_compare(const BtorBitVector& a, const BtorBitVector& b)
{
    if (a.width != b.width) return -1;

#ifdef BTOR_USE_GMP
    return mpz_cmp(a.val, b.val);
#else
    for (uint32_t i = 0; i < a.len; ++i)
    {
        if (a.bits[i] != b.bits[i])
        {
            return a.bits[i] > b.bits[i] ? 1 : -1;
        }
    }
    return 0;
#endif
}




inline BtorBitVectorPtr btor_bv_copy(const BtorBitVector& bv)
{
    const uint32_t bw = bv.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init_set(res->val, bv.val);  // deep copy
    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv.get();

    assert(res->len == bv.len);
    memcpy(res->bits, bv.bits, sizeof(BTOR_BV_TYPE) * bv.len);  // deep copy
    assert(btor_bv_compare(res, &bv) == 0);  // safe comparison

    return std::move(wrapper->bv_ptr);
#endif
}





inline BtorBitVectorPtr btor_bv_uext(const BtorBitVector& bv, uint32_t len)
{
    if (len == 0)
        return btor_bv_copy(bv);

    uint32_t bw = bv.width + len;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init_set(res->val, bv.val);
    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv.get();

    assert(res->len >= bv.len);
    memcpy(res->bits + (res->len - bv.len), bv.bits, sizeof(BTOR_BV_TYPE) * bv.len);

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}




inline BtorBitVectorPtr btor_bv_slice(const BtorBitVector& bv, uint32_t upper, uint32_t lower)
{
    assert(upper >= lower);
    assert(upper < bv.width);

    uint32_t bw = upper - lower + 1;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init(res->val);

    mpz_fdiv_r_2exp(res->val, bv.val, upper + 1);   // keep low bits [0..upper]
    mpz_fdiv_q_2exp(res->val, res->val, lower);     // shift right by `lower` bits

    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv.get();

    uint32_t j = 0;
    for (uint32_t i = lower; i <= upper; ++i)
        btor_bv_set_bit(*res, j++, btor_bv_get_bit(bv, i));

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}



inline BtorBitVectorPtr btor_bv_concat(const BtorBitVector& a, const BtorBitVector& b)
{
    uint32_t bw = a.width + b.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(a.width);
    res->width = bw;
    mpz_init(res->val);

    mpz_mul_2exp(res->val, a.val, b.width);
    mpz_add(res->val, res->val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);

    return res;
#else
    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv.get();

    int64_t j = res->len - 1;

    // Copy bits from b
    for (int64_t i = b->len - 1; i >= 0; --i)
        res->bits[j--] = b->bits[i];

    int64_t k = b->width % BTOR_BV_TYPE_BW;

    // Copy bits from a
    if (k == 0)
    {
        for (int64_t i = a->len - 1; i >= 0; --i)
            res->bits[j--] = a->bits[i];
    }
    else
    {
        ++j;
        BTOR_BV_TYPE v = res->bits[j];
        assert((v >> k) == 0);

        for (int64_t i = a->len - 1; i >= 0; --i)
        {
            v = v | (a->bits[i] << k);
            res->bits[j--] = v;
            v = a->bits[i] >> (BTOR_BV_TYPE_BW - k);
        }

        if (j == 0)
            res->bits[j] = v;
    }

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_ite(const BtorBitVector& cond,
  const BtorBitVector& then_bv,
  const BtorBitVector& else_bv)
{
    assert(then_bv.width == else_bv.width);

#ifdef BTOR_USE_GMP
    if (btor_bv_is_one(cond))
    return btor_bv_copy(then_bv);
    else
    return btor_bv_copy(else_bv);
#else
    assert(cond.len == 1);
    assert(then_bv.len == else_bv.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(then_bv.width);
    BtorBitVector* res = wrapper->bv.get();

    BTOR_BV_TYPE cc = btor_bv_get_bit(&cond, 0) ? ~static_cast<BTOR_BV_TYPE>(0) : 0;
    BTOR_BV_TYPE nn = ~cc;

    for (uint32_t i = 0; i < then_bv.len; ++i)
    res->bits[i] = (cc & then_bv.bits[i]) | (nn & else_bv.bits[i]);

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}



inline BtorBitVectorPtr btor_bv_eq(const BtorBitVector& a, const BtorBitVector& b)
{

    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    return mpz_cmp(a.val, b.val) == 0
           ? btor_bv_one(1)
           : btor_bv_zero(1);
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(1);
    BtorBitVector* res = wrapper->bv.get();

    bool is_equal = true;
    for (uint32_t i = 0; i < a->len; ++i)
    {
        if (a.bits[i] != b.bits[i])
        {
            is_equal = false;
            break;
        }
    }

    res->bits[0] = is_equal ? 1 : 0;

    assert(rem_bits_zero_dbg(res));
    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_xor(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    auto res = std::make_unique<BtorBitVector>(bw);
    res->width = bw;
    mpz_init(res->val);

    mpz_xor(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);

    return res;
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    BtorBitVector* res = wrapper->bv.get();

    for (uint32_t i = 0; i < a.len; ++i)
        res->bits[i] = a.bits[i] ^ b.bits[i];

    set_rem_bits_to_zero(res);
    assert(rem_bits_zero_dbg(res));

    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_mul(const BtorBitVector& a,
                             const BtorBitVector& b)
{
    assert(a.width == b.width);
    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    auto res = btor_bv_new(bw);
    mpz_mul(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);
    return res;
#else
    assert(a.len == b.len);

    if (bw <= 64)
    {
        uint64_t x = btor_bv_to_uint64(&a);
        uint64_t y = btor_bv_to_uint64(&b);
        return btor_bv_from_uint64(x * y, bw);
    }

    auto res = btor_bv_zero(bw);             // btor_bv_new(bw) with zeros

    for (uint32_t i = 0; i < bw; ++i)
    {
        BtorBitVectorPtr partial;

        if (btor_bv_get_bit(&b, i))
        {
            partial = btor_bv_copy(&a);
        }
        else
        {
            partial = btor_bv_zero(bw);
        }

        auto shifted = btor_bv_sll_uint64(partial.get(), i);
        auto added   = btor_bv_add(res.get(), shifted.get());

        res = std::move(added);              // 更新累计结果
    }

    return res;
#endif
}




inline BtorBitVectorPtr btor_bv_int64_to_bv(int64_t value, uint32_t bw)
{
    assert(bw > 0);

#ifdef BTOR_USE_GMP
    auto res = btor_bv_new(bw);
    mpz_init_set_si(res->val, value);
    mpz_fdiv_r_2exp(res->val, res->val, bw);
    return res;
#else
    auto res = btor_bv_zero(bw);

    // Fill with bits from value
    res->bits[res->len - 1] = static_cast<BTOR_BV_TYPE>(value);
    if (bw > BTOR_BV_TYPE_BW && res->len >= 2)
        res->bits[res->len - 2] = static_cast<BTOR_BV_TYPE>(value >> BTOR_BV_TYPE_BW);

    // Extend sign if value is negative
    if (value < 0 && bw > 64)
    {
        auto not_res = btor_bv_not(res.get());
        return not_res;
    }

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));

    return res;
#endif
}




inline BtorBitVectorPtr btor_bv_from_bool(bool val)
{
    auto res = btor_bv_zero(1);
    if (val) btor_bv_set_bit(*res, 0, 1);
    return res;
}

inline BtorBitVectorPtr btor_bv_ne(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    bool is_ne = mpz_cmp(a.val, b.val) != 0;
    return btor_bv_from_bool(is_ne);
#else
    assert(a.len == b.len);

    bool eq = true;
    for (uint32_t i = 0; i < a->len; i++)
    {
        if (a.bits[i] != b.bits[i])
        {
            eq = false;
            break;
        }
    }

    return btor_bv_from_bool(!eq);
#endif
}




inline BtorBitVectorPtr btor_bv_ones(uint32_t bw)
{
    assert(bw > 0);

#ifdef BTOR_USE_GMP
    auto res = btor_bv_one(bw);  // 1
    mpz_mul_2exp(res->val, res->val, bw);  // 1 << bw
    mpz_sub_ui(res->val, res->val, 1);     // (1 << bw) - 1
    return res;
#else
    auto zero = btor_bv_zero(bw);
    return btor_bv_not(zero.get());
#endif
}


inline BtorBitVectorPtr btor_bv_udiv(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    if (btor_bv_is_zero(b))
        return btor_bv_ones(bw);

    auto res = btor_bv_new(bw);
    mpz_fdiv_q(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);
    return res;
#else
    assert(a.len == b.len);

    BtorBitVector* raw_res = nullptr;
    udiv_urem_bv(a, b, &raw_res, nullptr);  // get quotient only
    assert(raw_res);
    return BtorBitVectorPtr(raw_res);      // wrap raw pointer into unique_ptr
#endif
}


inline BtorBitVectorPtr btor_bv_neg(const BtorBitVector& bv)
{
    uint32_t bw = bv.width;

#ifdef BTOR_USE_GMP
    auto res = btor_bv_not(bv);
    mpz_add_ui(res->val, res->val, 1);               // res = ~bv + 1
    mpz_fdiv_r_2exp(res->val, res->val, bw);         // truncate to bw
    return res;
#else
    auto not_bv = btor_bv_not(bv);
    auto one = btor_bv_from_uint64(1, bw);
    return btor_bv_add(*not_bv, *one);               // return ~bv + 1
#endif
}



inline BtorBitVectorPtr btor_bv_sub(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    auto res = btor_bv_new(a.width);
    mpz_sub(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, a.width);
    return res;
#else
    auto neg_b = btor_bv_neg(b);
    auto res = btor_bv_add(a, *neg_b);
    return res;
#endif
}



inline bool btor_bv_is_true(const BtorBitVector& bv)
{
    assert(bv.width == 1);
    return btor_bv_get_bit(bv, 0);
}

inline bool btor_bv_is_false(const BtorBitVector& bv)
{
    assert(bv.width == 1);
    return !btor_bv_get_bit(bv, 0);
}



inline BtorBitVectorPtr btor_bv_sext(const BtorBitVector& bv, uint32_t len)
{
    if (len == 0)
        return btor_bv_copy(bv);

    uint32_t bw = bv.width;

#ifdef BTOR_USE_GMP
    auto res = btor_bv_copy(bv);
    res->width += len;
    if (btor_bv_get_bit(bv, bw - 1)) {
        for (uint32_t i = bw; i < bw + len; ++i)
            mpz_setbit(res->val, i);
    }
    return res;
#else
    auto prefix = btor_bv_get_bit(&bv, bw - 1)
                    ? btor_bv_ones(len)
                    : btor_bv_zero(len);

    auto res = btor_bv_concat(*prefix, bv);
    return res;
#endif
}


inline BtorBitVectorPtr btor_bv_ult(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    return mpz_cmp(a.val, b.val) < 0 ? btor_bv_one(1) : btor_bv_zero(1);
#else
    assert(a.len == b.len);
    for (uint32_t i = 0; i < a.len; ++i)
    {
        if (a.bits[i] != b.bits[i])
        {
            is_less = a.bits[i] < b.bits[i];
            break;
        }
    }

    auto res = btor_bv_zero(1);
    btor_bv_set_bit(res.get(), 0, is_less);
    assert(rem_bits_zero_dbg(res.get()));
    return res;
#endif
}



inline BtorBitVectorPtr btor_bv_ulte(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    return mpz_cmp(a.val, b.val) <= 0
             ? btor_bv_one(1)
             : btor_bv_zero(1);
#else
    assert(a.len == b.len);
    bool is_le = true;

    for (uint32_t i = 0; i < a.len; ++i)
    {
        if (a.bits[i] != b.bits[i])
        {
            is_le = a.bits[i] < b.bits[i];
            break;
        }
    }

    auto res = btor_bv_zero(1);
    btor_bv_set_bit(res.get(), 0, is_le);
    assert(rem_bits_zero_dbg(res.get()));
    return res;
#endif
}




inline BtorBitVectorPtr btor_bv_ugt(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    return mpz_cmp(a.val, b.val) > 0
             ? btor_bv_one(1)
             : btor_bv_zero(1);
#else
    assert(a.len == b.len);
    bool is_gt = false;

    for (uint32_t i = 0; i < a.len; ++i)
    {
        if (a.bits[i] != b.bits[i])
        {
            is_gt = a.bits[i] > b.bits[i];
            break;
        }
    }

    auto res = btor_bv_zero(1);
    btor_bv_set_bit(res.get(), 0, is_gt);
    assert(rem_bits_zero_dbg(res.get()));
    return res;
#endif
}




inline BtorBitVectorPtr btor_bv_ugte(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    return mpz_cmp(a.val, b.val) >= 0
             ? btor_bv_one(1)
             : btor_bv_zero(1);
#else
    assert(a.len == b.len);
    bool is_ge = true;

    for (uint32_t i = 0; i < a.len; ++i)
    {
        if (a.bits[i] != b.bits[i])
        {
            is_ge = a.bits[i] > b.bits[i];
            break;
        }
    }

    auto res = btor_bv_zero(1);
    btor_bv_set_bit(res.get(), 0, is_ge);
    assert(rem_bits_zero_dbg(res.get()));
    return res;
#endif
}




inline BtorBitVectorPtr btor_bv_slt(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;
    bool msb_a = btor_bv_get_bit(a, bw - 1);
    bool msb_b = btor_bv_get_bit(b, bw - 1);

    if (msb_a && !msb_b)
    {
        return btor_bv_one(1);
    }
    else if (!msb_a && msb_b)
    {
        return btor_bv_zero(1);
    }
    else
    {
        return btor_bv_ult(a, b);
    }
}



inline BtorBitVectorPtr btor_bv_slte(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;
    bool msb_a = btor_bv_get_bit(a, bw - 1);
    bool msb_b = btor_bv_get_bit(b, bw - 1);

    if (msb_a && !msb_b)
    {
        return btor_bv_one(1);
    }
    else if (!msb_a && msb_b)
    {
        return btor_bv_zero(1);
    }
    else
    {
        return btor_bv_ulte(a, b);
    }
}




inline BtorBitVectorPtr btor_bv_sgt(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;
    bool msb_a = btor_bv_get_bit(a, bw - 1);
    bool msb_b = btor_bv_get_bit(b, bw - 1);

    if (msb_a && !msb_b)
    {
        return btor_bv_zero(1);
    }
    else if (!msb_a && msb_b)
    {
        return btor_bv_one(1);
    }
    else
    {
        return btor_bv_ugt(a, b);
    }
}




inline BtorBitVectorPtr btor_bv_sgte(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;
    bool msb_a = btor_bv_get_bit(a, bw - 1);
    bool msb_b = btor_bv_get_bit(b, bw - 1);

    if (msb_a && !msb_b)
    {
        return btor_bv_zero(1);
    }
    else if (!msb_a && msb_b)
    {
        return btor_bv_one(1);
    }
    else
    {
        return btor_bv_ugte(a, b);
    }
}



inline BtorBitVectorPtr btor_bv_xnor(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;

#ifdef BTOR_USE_GMP
    auto res = btor_bv_new(bw);
    mpz_xor(res->val, a.val, b.val);
    mpz_com(res->val, res->val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);
    return res;
#else
    assert(a.len == b.len);

    auto wrapper = std::make_unique<BtorBitVectorWrapper>(bw);
    auto& res = wrapper->bv;

    for (uint32_t i = 0; i < a.len; ++i)
        res->bits[i] = ~(a.bits[i] ^ b.bits[i]);

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));

    return std::move(wrapper->bv_ptr);
#endif
}


inline BtorBitVectorPtr btor_bv_urem(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

#ifdef BTOR_USE_GMP
    uint32_t bw = a.width;

    if (btor_bv_is_zero(b))
        return btor_bv_copy(a);

    auto res = btor_bv_new(bw);
    mpz_fdiv_r(res->val, a.val, b.val);
    mpz_fdiv_r_2exp(res->val, res->val, bw);

    return res;
#else
    assert(a.len == b.len);

    BtorBitVector* res_raw = nullptr;
    udiv_urem_bv(a, b, nullptr, &res_raw);
    assert(res_raw);

    return BtorBitVectorPtr(res_raw);
#endif
}




inline BtorBitVectorPtr btor_bv_sdiv(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint32_t bw = a.width;
    bool is_signed_a = btor_bv_get_bit(a, bw - 1);
    bool is_signed_b = btor_bv_get_bit(b, bw - 1);

    if (is_signed_a && !is_signed_b)
    {
        auto neg_a = btor_bv_neg(a);
        auto div = btor_bv_udiv(*neg_a, b);
        return btor_bv_neg(*div);
    }
    else if (!is_signed_a && is_signed_b)
    {
        auto neg_b = btor_bv_neg(b);
        auto div = btor_bv_udiv(a, *neg_b);
        return btor_bv_neg(*div);
    }
    else if (is_signed_a && is_signed_b)
    {
        auto neg_a = btor_bv_neg(a);
        auto neg_b = btor_bv_neg(b);
        return btor_bv_udiv(*neg_a, *neg_b);
    }
    else
    {
        return btor_bv_udiv(a, b);
    }
}



inline BtorBitVectorPtr btor_bv_srem(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    const uint32_t bw = a.width;
    const bool is_signed_a = btor_bv_get_bit(a, bw - 1);
    const bool is_signed_b = btor_bv_get_bit(b, bw - 1);

    if (is_signed_a && !is_signed_b)
    {
        auto neg_a = btor_bv_neg(a);
        auto rem = btor_bv_urem(*neg_a, b);
        return btor_bv_neg(*rem);
    }
    else if (!is_signed_a && is_signed_b)
    {
        auto neg_b = btor_bv_neg(b);
        return btor_bv_urem(a, *neg_b);
    }
    else if (is_signed_a && is_signed_b)
    {
        auto neg_a = btor_bv_neg(a);
        auto neg_b = btor_bv_neg(b);
        auto rem = btor_bv_urem(*neg_a, *neg_b);
        return btor_bv_neg(*rem);
    }
    else
    {
        return btor_bv_urem(a, b);
    }
}



inline BtorBitVectorPtr btor_bv_srl_uint64(const BtorBitVector& a, uint64_t shift)
{
    auto res = btor_bv_new(a.width);
    if (shift >= a.width) return res;

#ifdef BTOR_USE_GMP
    mpz_fdiv_q_2exp(res->val, a.val, shift);
#else
    const uint32_t skip = shift / BTOR_BV_TYPE_BW;
    const uint32_t k = shift % BTOR_BV_TYPE_BW;

    BTOR_BV_TYPE v = 0;
    for (uint32_t i = 0, j = skip; i < a.len && j < a.len; ++i, ++j)
    {
        v = (k == 0) ? a.bits[i] : v | (a.bits[i] >> k);
        res->bits[j] = v;
        v = (k == 0) ? a.bits[i] : a.bits[i] << (BTOR_BV_TYPE_BW - k);
    }

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));
#endif

    return res;
}



//TODO
#ifdef BTOR_USE_GMP
static uint32_t
inline get_limb (const BtorBitVector *bv,
          mp_limb_t *limb,
          uint32_t nbits_rem,
          bool zeros)
{
  /* GMP normalizes the limbs, the left most (most significant) is never 0 */
  uint32_t i, n_limbs, n_limbs_total;
  mp_limb_t res = 0u, mask;

  n_limbs = mpz_size (bv->val);

  /* for leading zeros */
  if (zeros)
  {
    *limb = n_limbs ? mpz_getlimbn (bv->val, n_limbs - 1) : 0;
    return n_limbs;
  }

  /* for leading ones */
  n_limbs_total = bv->width / mp_bits_per_limb + (nbits_rem ? 1 : 0);
  if (n_limbs != n_limbs_total)
  {
    /* no leading ones, simulate */
    *limb = nbits_rem ? ~(~((mp_limb_t) 0) << nbits_rem) : ~((mp_limb_t) 0);
    return n_limbs_total;
  }
  mask = ~((mp_limb_t) 0) << nbits_rem;
  for (i = 0; i < n_limbs; i++)
  {
    res = mpz_getlimbn (bv->val, n_limbs - 1 - i);
    if (nbits_rem && i == 0)
    {
      res = res | mask;
    }
    res = ~res;
    if (res > 0) break;
  }
  *limb = res;
  return n_limbs - i;
}
#else
static uint32_t
inline get_limb (const BtorBitVector *bv,
          BTOR_BV_TYPE *limb,
          uint32_t nbits_rem,
          bool zeros)
{
  uint32_t i;
  BTOR_BV_TYPE res = 0u, mask;

  /* for leading zeros */
  if (zeros)
  {
    for (i = 0; i < bv->len; i++)
    {
      res = bv->bits[i];
      if (res > 0) break;
    }
  }
  /* for leading ones */
  else
  {
    mask = ~((BTOR_BV_TYPE) 0) << nbits_rem;
    for (i = 0; i < bv->len; i++)
    {
      res = bv->bits[i];
      if (nbits_rem && i == 0)
      {
        res = res | mask;
      }
      res = ~res;
      if (res > 0) break;
    }
  }

  *limb = res;
  return bv->len - i;
}
#endif



static uint32_t
inline get_num_leading (const BtorBitVector *bv, bool zeros)
{
  assert (bv);

  uint32_t res = 0, nbits_pad;
  /* The number of limbs required to represent the actual value.
   * Zero limbs are disregarded. */
  uint32_t n_limbs;
  /* Number of limbs required when representing all bits. */
  uint32_t n_limbs_total;
  /* The number of bits that spill over into the most significant limb,
   * assuming that all bits are represented). Zero if the bit-width is a
   * multiple of n_bits_per_limb. */
  uint32_t nbits_rem;
  uint32_t nbits_per_limb;
#ifdef BTOR_USE_GMP
  mp_limb_t limb;
#else
  BTOR_BV_TYPE limb;
#endif

#ifdef BTOR_USE_GMP
  nbits_per_limb = mp_bits_per_limb;
#else
  nbits_per_limb = BTOR_BV_TYPE_BW;
#endif

  nbits_rem = bv->width % nbits_per_limb;

  n_limbs = get_limb (bv, &limb, nbits_rem, zeros);
  if (n_limbs == 0) return bv->width;

#if defined(__GNUC__) || defined(__clang__)
  res = nbits_per_limb == 64 ? __builtin_clzll (limb) : __builtin_clz (limb);
#else
  res = clz_limb (nbits_per_limb, limb);
#endif
  n_limbs_total = bv->width / nbits_per_limb + 1;
  nbits_pad     = nbits_per_limb - nbits_rem;
  res += (n_limbs_total - n_limbs) * nbits_per_limb - nbits_pad;
  return res;
}

//TODO
inline uint32_t btor_bv_get_num_leading_zeros (const BtorBitVector *bv)
{
  return get_num_leading (bv, true);
}



inline static bool shift_is_uint64(const BtorBitVector* b, uint64_t* res)
{
    assert(b);
    assert(res);

    if (b->width <= 64)
    {
        *res = btor_bv_to_uint64(b);
        return true;
    }

    uint64_t zeroes = btor_bv_get_num_leading_zeros(b);
    if (zeroes < b->width - 64) return false;

    uint32_t upper = (zeroes < b->width) ? b->width - 1 - zeroes : 0;
    auto shift = btor_bv_slice(*b, upper, 0);
    assert(shift->width <= 64);

    *res = btor_bv_to_uint64(shift.get());
    return true;
}




inline BtorBitVectorPtr btor_bv_srl(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint64_t ushift;
    if (shift_is_uint64(&b, &ushift))
    {
        return btor_bv_srl_uint64(a, ushift);
    }

    return btor_bv_zero(a.width);
}



inline BtorBitVectorPtr btor_bv_sra(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    if (btor_bv_get_bit(a, a.width - 1))
    {
        auto not_a = btor_bv_not(a);
        auto not_a_srl_b = btor_bv_srl(*not_a, b);
        return btor_bv_not(*not_a_srl_b);
    }
    else
    {
        return btor_bv_srl(a, b);
    }
}

inline BtorBitVectorPtr btor_bv_sll_uint64(const BtorBitVector& a, uint64_t shift)
{
    uint32_t bw = a.width;
    auto res = btor_bv_new(bw);
    if (shift >= bw) return res;

#ifdef BTOR_USE_GMP
    mpz_mul_2exp(res->val, a.val, shift);
    mpz_fdiv_r_2exp(res->val, res->val, bw);
#else
    uint32_t skip = shift / BTOR_BV_TYPE_BW;
    uint32_t k = shift % BTOR_BV_TYPE_BW;

    BTOR_BV_TYPE v = 0;

    for (int64_t i = static_cast<int64_t>(a.len) - 1,
                 j = static_cast<int64_t>(res->len) - 1 - skip;
         i >= 0 && j >= 0;
         --i, --j)
    {
        v = (k == 0) ? a.bits[i] : v | (a.bits[i] << k);
        res->bits[j] = v;
        v = (k == 0) ? a.bits[i] : a.bits[i] >> (BTOR_BV_TYPE_BW - k);
    }

    set_rem_bits_to_zero(res.get());
    assert(rem_bits_zero_dbg(res.get()));
    assert(check_bits_sll_dbg(&a, res.get(), shift));
#endif

    return res;
}

inline BtorBitVectorPtr btor_bv_sll(const BtorBitVector& a, const BtorBitVector& b)
{
    assert(a.width == b.width);

    uint64_t ushift;
    if (shift_is_uint64(&b, &ushift))
    {
        return btor_bv_sll_uint64(a, ushift);
    }

    return btor_bv_new(a.width);
}



#endif