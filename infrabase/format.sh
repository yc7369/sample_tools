#! /bin/bash

formatDir() {
  for f in $(find $1 -name "*.h" -or  -name "*.c" -or -name "*.cc" -or -name "*.cpp" -type f)
  do
  #  --break-blocks
    clang-format -style=file -i $f
    # astyle --style=java --indent=spaces=4 --mode=c $f
    orig="$f.orig"
    # echo $orig
    if [ -f $orig ]; then
      # echo $orig
      rm $orig
    fi
  done
}

formatDir ./

