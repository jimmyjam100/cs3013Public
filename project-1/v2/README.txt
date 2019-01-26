=== Compiling ===

There is an included Makefile, but you can also run the following commands

$ gcc -pthread main.c -o mc2

To compile the program with the Makefile, run

$ make

We also support ($ make all), but it's no different than the previous commands.

Running ($ make clean) will delete the mc2 executable that was compiled.

=== Running ===

You can run the program with

$ ./mc2

=== Limitations ===

- You can add commands up to 999 characters long
- Commands can have up to 98 arguments
- You'll notice that if you have a command running in the background, and you exit from Mid-Day Commander ( eithere
  with the e command or EOF), then Mid-Day Commander will let you exit the program.

=== Testing ===

There are files included in the test-inputs directory and the test-outputs directory.

Each file in test-inputs has a corresponding file in test-outputs. If you pipe a file
from test-inputs into Mid-Day commander, you should expect a similar output like
the one in test-outputs.

For example:

$ ./mc2 < test-inputs/1.txt

should resemble

$ cat test-outputs/1.txt
