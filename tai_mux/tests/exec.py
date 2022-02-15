#!/usr/bin/env python

import sys
import time


def main():
    if len(sys.argv) != 2:
        sys.exit(1)

    arg = sys.argv[1]

    if arg == "list":
        print("\n".join(str(v) for v in range(1, 8)))
        return

    time.sleep(0.5)

    print("libtai-a.so")


if __name__ == "__main__":
    main()
