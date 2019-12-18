#!/bin/bash

set -e

BENCHMARK=$1
OUTPUT=$2
VALGRIND=$3
DOT=$4
GPROF2DOT=$5
BENCHMARK_NAME=$(basename $BENCHMARK)
BENCHMARKS=$($BENCHMARK --benchmark_list_tests)
TEMPDIR=$(mktemp -d)
echo $TEMPDIR
pushd $TEMPDIR
for FILTER in $BENCHMARKS; do
  FILTER_PRIME=$(echo $FILTER | tr / -)
  echo "Generate profile for $FILTER"
  $VALGRIND --tool=callgrind $BENCHMARK --benchmark_filter=$FILTER$
  echo "Convert $FILTER to image"
  $GPROF2DOT --format=callgrind --output=$BENCHMARK_NAME-$FILTER_PRIME.dot callgrind.out.[0-9]*
  $DOT -Tsvg $BENCHMARK_NAME-$FILTER_PRIME.dot -o "$BENCHMARK_NAME-$FILTER_PRIME.svg"
  mv callgrind.out.[0-9]* callgrind.out.$BENCHMARK_NAME-$FILTER_PRIME
  tar zcf $(basename $OUTPUT) *.svg *.dot callgrind.out.*
done
popd
mv $TEMPDIR/$(basename $OUTPUT) $OUTPUT
