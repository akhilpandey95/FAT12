# FAT12

### What is this about ?

`This is a simulation of a how a FAT12 system looks like. There are certain things to be kept in mind while studying the code for understanding what is it about.
  - The `data.txt` is a file that contains information pertaining to the FILE system simulation. Aspects like creation, modification etc, are included in the `data.txt` file.
  - The program is setup in such a way that it parses information from `data.txt` and then follows the cycle of instructions in order to simulate a FAT12 system.
  - `.` and `..` are included in the File directory of the FAT12 system
  - Since this is a simulation there is a constraint on the disk space, 2MB
  - The directory can contain a maximum of 12 files, after which the files are allocated onto the next block

### How to run ?

First create the binary using the `make` command
```shell
make
```

Now just run the binary
```shell
./fat12
```

### License
[Mozilla Public License](https://github.com/akhilpandey95/FAT12/blob/master/LICENSE)

### Contribute
Please raise an [Issue](https://github.com/akhilpandey95/FAT12/issues/new), if there is any technical discrepency either in the codebase or during the simulation of the FAT system

### Note
The intention of this repository and the codebase is not to help you skip your assignment or easing off the effort put in completion of it. If you are a student, PLEASE DONOT COPY THE SOURCE CODE AND USE THE SAME FOR YOUR ASSIGNMENT.

### Author
[Akhil Pandey](https://github.com/akhilpandey95)
