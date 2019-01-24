=== Project Structure ===
There are folders for mc0, mc1, and mc2, named v0, v1, and v2 (respectively).
This folder contains files pertaining to all versions of Mid-Day Commander.

=== Compiling ===

There is a README.txt file in each folder with instructions for compilation
but you can also use the Makefile in each directory, or use the Makefile
in the project directory. All Makefiles support the following commands:

$ make
$ make clean
$ make all

(default): builds mc0, mc1, and mc2
all: builds mc0, mc1, mc2
clean: deletes the mc0, mc1, and mc2 executables.

=== Features ===

For versions of Mid-Day commander that support user-added commands (mc1 and
mc2), then you can add commands up to 999 characters long, with up to 98
arguments in the command.
