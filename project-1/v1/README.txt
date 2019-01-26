=== Compiling ===

There is an included Makefile, but you can also run the following commands

$ gcc main.c -o mc1

To compile the program with the Makefile, run

$ make

We also support ($ make all), but it's no different than the previous commands.

Running ($ make clean) will delete the mc1 executable that was compiled.

=== Running ===

You can run the program with

$ ./mc1

=== Limitations ===

- You can add commands up to 999 characters long
- Commands can have up to 98 arguments

=== Testing ===

There are files included in the test-inputs directory and the test-outputs directory.

Each file in test-inputs has a corresponding file in test-outputs. If you pipe a file
from test-inputs into Mid-Day commander, you should expect a similar output like
the one in test-outputs.

For example:

$ ./mc1 < test-inputs/1.txt

should resemble

$ cat test-outputs/1.txt
