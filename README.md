# Project 1: System Inspector

In this project proc will be used to get information based outside of a kernel. Open(), close(), and read() are used to read and use file data. 

## Building

To compile and run:

```bash
make
./inspector
```

## Program Options

```bash
$ ./inspector -h
Usage: ./inspector [-ho] [-i interval] [-p procfs_dir]

Options:
    * -h              Display help/usage information
    * -i interval     Set the update interval (default: 1000ms)
    * -p procfs_dir   Set the expected procfs mount point (default: /proc)
    * -o              Operate in one-shot mode (no curses or live updates)
```

## Included Files

* **procfs.c** -- Major functions for pfs
* **util.c** -- Helper functions (open, close, lineread)

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'

# Run a test case in gdb:
make test run=4 debug=on
```

If you are satisfied with the state of your program, you can also run the test cases on the grading machine. Check your changes into your project repository and then run:

```
make grade
```

## Demo Run

<img width="558" alt="test_img" src="https://user-images.githubusercontent.com/60202581/156500672-cac321ea-fa0c-45b8-aaad-5a6c8fdb7b10.png">

