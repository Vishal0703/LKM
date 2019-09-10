Refer to part B of `Assignment_1_Kernel.pdf` for problem statement.

For Part B - Assignment 1, LKM is `partb_A1.c` and the corresponding input giving file is `input1.c`.

For Part B - Assignment 2, LKM is `partb_A2.c` and the corresponding input giving file is `input2.c`.

input1.c and input2.c are normal C files, so compile it with `gcc input1.c -o input1`.(similarly for `input2.c`)

`partb_A1.c` and `partb_A2.c` are LKM modules, and Makefile has been added.

To compile them, run `make` into cloned directory, and insert the LKM 1 into kernel via `sudo insmod partb_A1.ko` and LKM 2 into kernel via `sudo insmod partb_A2.ko`

