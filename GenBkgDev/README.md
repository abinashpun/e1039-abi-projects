# GenBkgDev
A simple module for generating the background cadidates from simulation, which can be used for pileup purpose. One can add more variables/branches in the source code as needed. 

## Source Files

All source files are placed in `src/`.
You use the following commands to compile them.

```
source setup.sh
cmake-this
make-this
```

## Analysis Procedure
```
cd macros
root -b -q 'Fun4Sim_genbkg.C(100)'
``` 
## Advanced Usage

### `cmake-this`

You should execute this function when you add/delete files under `src/`.
It is a shell function defined in `setup.sh`.
It calls 'cmake' with a proper configuration.
It cleans up all files in the build & install directories.

### `make-this`

You should execute this function when you modify files under `src/`.
It is a shell function defined in `setup.sh`.
It calls 'make install' with a proper configuration.

## Author
Abinash Pun
