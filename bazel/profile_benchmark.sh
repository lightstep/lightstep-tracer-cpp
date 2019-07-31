#!/bin/bash

set -e

BENCHMARK=$1
OUTPUT=$2
BENCHMARK_NAME=$(basename $BENCHMARK)
BENCHMARKS=$($BENCHMARK --benchmark_list_tests)
TEMPDIR=$(mktemp -d)
echo $TEMPDIR
pushd $TEMPDIR
for FILTER in $BENCHMARKS; do
  echo "Generate profile for $FILTER"
  valgrind --tool=callgrind $BENCHMARK --benchmark_filter=$FILTER$
  echo "Convert $FILTER to image"
  gprof2dot --format=callgrind --output=out.dot callgrind.out.*
  FILTER_PRIME=$(echo $FILTER | tr / -)
  dot -Tpng out.dot -o "$BENCHMARK_NAME-$FILTER_PRIME.png"
  rm callgrind.out.*
  rm *.dot
  tar zcf $(basename $OUTPUT) *.png
done
popd
mv $TEMPDIR/$(basename $OUTPUT) $OUTPUT
