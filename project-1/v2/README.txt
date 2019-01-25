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
