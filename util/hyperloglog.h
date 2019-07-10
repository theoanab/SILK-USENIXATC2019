/*
 * Copyright (c) 2014 Nutanix Inc.  All rights reserved.
 *
 * Author: rickard.faith@nutanix.com
 *
 * This class implements the hyperloglog algorithm for estimating the
 * cardinality of a set using a small amount of memory unrelated to the
 * cardinality of the set. The implementation is based on these papers:
 *
 * Flajolet, Philippe; Fusy, Éric; Gandouet, Olivier; Meunier, Frédéric
 * (2007). "HyperLogLog: the analysis of a near-optimal cardinality estimation
 * algorithm". In AOFA ’07: Proceedings of the 2007 International Conference on
 * the Analysis of Algorithms.
 *
 * Heule, Stefan; Nunkesser, Marc; Hall, Alexander (2013). "HyperLogLog in
 * practice: algorithmic engineering of a state of the art cardinality
 * estimation algorithm." In Proceedings of the EDBT 2013 Conference, Genoa,
 * Italy, March 18-22.
 *
 * Brief Introduction to the HyperLogLog Algorithm:
 *
 * For a stream of random numbers if we observed a number with 'n' leading 0s
 * in its binary representation then there is a good chance that we saw at
 * least 2^(n + 1) unique numbers. If we track the highest such 'n' for all
 * numbers observed, we can guess the cardinality of the set to be 2^(n + 1).
 *
 * This guess can be made more accurate if we have multiple such tracking
 * buckets, shard the random numbers from the stream to one of these buckets,
 * and then compute the overall cardinality as the normalized harmonic mean of
 * the cardinality values estimated by each bucket.
 *
 * This implementation uses the last few bits of the random number to select
 * the bucket and the high-order bits to determine n+1 (rho=n+1 in the papers
 * and in our implementation).
 *
 * Discussion:
 *
 * The exact cardinality of a set can only be determined using an amount of
 * memory proportional to the cardinality of the set. The HyperLogLog
 * algorithm uses a fixed amount of memory, independent of the cardinality of
 * the set, to provide an estimate of the set's cardinality. Using more memory
 * results in a more accurate estimate, but using only 1KB of memory produces
 * an estimate within about 3-10% of the exact value. Further, the cardinality
 * of the set can be quite large, ranging in this implementation from 2^46 to
 * 2^56 depending on the value of num_sharding_bits. So, if we are counting
 * 4KB blocks on a vdisk with num_sharding_bits <= 12, the vdisk should be
 * smaller than an exabyte in size (and the granularity parameter should be
 * 4096).
 *
 * The num_sharding_bits value of the HyperLogLog class specifies both how
 * accurate the cardinality estimate will be, as well as how much memory will
 * be used for each interval. The memory used is m=2^b, where m is given in
 * bytes and b is the num_sharding_bits value used during instantiation. Our
 * implementation uses 'm' sharding buckets, each of which is 8-bits in size.
 * Each bucket stores a value for rho, which can range from 1 to (64 - 'b').
 *
 * The typical relative error is given as e = +/- 1.04/sqrt(m), but in
 * practice the error is bounded by about 3 times this value because of
 * differences in hashing and the randomness of the input stream. Using the 3e
 * value, the memory vs. error tradeoff is seen in the following table:
 *
 * num_sharding_bits   memory (bytes)    3e (%)
 *
 *  4                       16             78
 *  6                       64             40
 *  8                      256             20
 * 10                     1024             10
 * 12                     4096              4.9
 * 14                    16384              2.4
 * 16                    65536              1.2
 *
 * The Heule paper describes three classes of improvements. We have used the
 * 64-bit hash improvement. We have computed the bias correction for
 * num_sharding_bits=8, but it did not provide a significant improvement over
 * the default linear counting correction, so we did not implement it. The
 * sparse representation is not needed for num_sharding_bits=8, so we did not
 * implement it.
 *
 * Example usage:
 *
 * #include "util/base/hyperloglog.h"
 *
 * const vector<int> intervals_secs({120, 3600});
 * HyperLogLog::Ptr hll = make_shared<HyperLogLog>(WallTime:;NowUsecs,
 *                                                 intervals_secs);
 *
 * hll->Add(offset, length);
 *
 * vector<int64_t> cardinality = hll->Cardinality();
 *
 * In an alarm handler:  hll->AlarmHandler(WallTime::NowUsecs())
 *
 */

#ifndef _UTIL_BASE_HYPERLOGLOG_H_
#define _UTIL_BASE_HYPERLOGLOG_H_

#include <stdint.h>
#include <vector>

namespace rocksdb {

class HyperLogLog {
 public:

  // Create the HyperLogLog object so that cardinality can be tracked within 1
  // or more intervals. Current time is in microseconds. Interval times are in
  // seconds. The num_sharding_bits value must be between 4 and 16, inclusive;
  // using a larger value will cause the object to consume more memory. The
  // granularity parameter specifies the size to which element values will be
  // rounded (using integer division).
  HyperLogLog(int num_sharding_bits);

  ~HyperLogLog() = default;

  // Add all the elements based on the hash value. This can be used by the
  // caller to avoid computing the hash more than once if multiple HyperLogLog
  // instances must be updated. For VDiskCacheSizeEstimator, we return true if
  // any bucket was changed; this indicates the cardinality has likely changed
  // and should be recomputed if needed.
  bool AddHash(int64_t hash);

  static int64_t MergedEstimate(const std::vector<HyperLogLog*>& hll_vector);

 private:

  // Compute an estimate of cardinality for a set of counters. This is an
  // internal helper method.
  static int64_t Estimate(const std::vector<int8_t>& counters, bool correct,
                        double alpha_num_buckets2);

  // The number of bits (b from Flajolet) used for sharding the input into
  // buckets. More bits will use more memory, but provide a more accurate
  // estimate of cardinality.
  const int num_sharding_bits_;

  // Precompute the number of buckets (m=2^b from Flajolet).
  const int num_buckets_;

  // Precompute the mask to determine the bucket.
  const int bucket_mask_;

  // Stores the maximum bit offset for each hash for each bucket in each
  // interval.
  std::vector<int8_t> counters_;

  // The value of alpha is given by Flajolet at the top of Figure 3 (p. 140).
  // Here, we store alpha * num_buckets_^2, which is used in Estimator().
  double alpha_num_buckets2_;
};

} // namespace

#endif // _UTIL_BASE_HYPERLOGLOG_H_