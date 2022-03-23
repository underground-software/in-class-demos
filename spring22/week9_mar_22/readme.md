## Week 9 example code

This module demos platform drivers, mutexes, and the devm system for managing resources.

You can build it with `make build` load the modules with `make load` (unload with `make unload` and clean with `make clean`)

It manifests as two files in `/dev`: `/dev/num-inc` and `/dev/num-dec`. Reading from either of them returns the value
of a shared counter (you can use `dd if=/dev/num-??? count=1 2>/dev/null` to read from one of them (it won't print a newline so the
output appears before your next terminal prompt). Writing to `num-inc` increments the shared count (`echo "test" > /dev/num-inc`)
and writing to `num-dec` decrements it (`echo "test" > /dev/num-dec`).
