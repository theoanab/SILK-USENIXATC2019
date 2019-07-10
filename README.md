## SILK: Preventing Latency Spikes in Log-Structured Merge Key-Value Stores

This is the SILK prototype for the USENIX ATC'19 submission. https://www.usenix.org/conference/atc19/presentation/balmau

SILK is built as an extension of RocksDB https://rocksdb.org/

Useful files for running benchmarks:

The benchmrks are run through the standard RocksDB db_bench https://github.com/facebook/rocksdb/wiki/Benchmarking-tools 

db_bench_tool.cc is the main db_bench file. Contains all the tests, including the YCSB benchmark and LongPeakTest, which fluctuates load peaks and valleys.

Useful files for SILK implementation:
compaction_picker.cc
db.h
db_impl.cc
db_impl.h
db_impl_compaction_flush.cc
thread_status_impl.cc


