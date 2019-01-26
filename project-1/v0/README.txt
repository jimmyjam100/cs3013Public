=== Compiling ===

There is an included Makefile, but you can also run the following commands

$ gcc main.c -o mc0

To compile the program with the Makefile, run

$ make

We also support ($ make all), but it's no different than the previous commands.

Running ($ make clean) will delete the mc2 executable that was compiled.

=== Running ===

You can run the program with

$ ./mc0

=== Limitations ===

- There are currently no limitations that are not present in the spec sheet for the assignment.

=== Testing ===

There are files included in the test-inputs directory and the test-outputs directory.

Each file in test-inputs has a corresponding file in test-outputs. If you pipe a file
from test-inputs into Mid-Day commander, you should expect a similar output like
the one in test-outputs.

For example:

$ ./mc0 < test-inputs/1.txt

should resemble

$ cat test-outputs/1.txt
