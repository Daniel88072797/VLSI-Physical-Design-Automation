# Homework 4 Global Placement
Follow the tips in `GlobalPlacer.cpp` to get a placement with minimum HPWL.

## How to Compile
In this directory, enter the following command:
```sh
$ make
```
It will generate the executable file `hw4` in the directory `HW4/bin`.

If you want to remove it, please enter the following command:
```sh
$ make clean
```

## How to Run
Usage:
```
$ ./hw4 <.aux file> <.gp.pl file>
```
For example,
```sh
$ ./hw4 ../testcase/public1/public1.aux ../output/public1.gp.pl
```

## How to Test
In this directory, enter the following command:
```sh
$ make test ${name}
```
It will test on ${name} and verify the result.

For example, test on public1.
```sh
$ make test public1
```
