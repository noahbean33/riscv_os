#!/bin/bash

HEADER_FILE=include/build_time.h

# Generate time in UTC
read Y M D H Min S <<< $(date -u '+%Y %-m %-d %-H %-M %-S')

mkdir -p $(dirname "$HEADER_FILE")

cat <<EOF > "$HEADER_FILE"
// Auto-generated build time
#pragma once

#define BUILD_YEAR   $Y
#define BUILD_MONTH  $M
#define BUILD_DAY    $D
#define BUILD_HOUR   $H
#define BUILD_MINUTE $Min
#define BUILD_SECOND $S
EOF

echo "[build_time.h] generated: $Y-$M-$D $H:$Min:$S UTC"