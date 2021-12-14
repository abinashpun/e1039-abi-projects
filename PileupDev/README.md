# PileupDev
A simple module for the piling up the background events from simulation. One can add more variables/branches in the source code as needed. 

## Source Files

All source files are placed in `src/`.
You use the following commands to compile them.

```
source setup.sh
cmake-this
make-this
```

## Analysis Procedure
1. Requriemet: 
	- ROOT file with background candidate information is needed. Separately module to generate such files are under developement but general users are not expected to produce such files by themselves. Such files are currently provided under the `/pnfs/e1039/persistent/users/apun/bkg_study/e1039pythiaGen_17Nov21/*root` location. 
 	- NIM3 intensity profile from E906 data (from road set 67) are provided as `e906_rf00.root` under `macros` directory
1. Run the pileup macro with desired methods of pileup
	- with distribution in the form of histogram (default) from E906 NIM3 intensity profile. You can choose the inhibit threshold (in intensity distribution) and proton conversion factor (following same convention as in E906 data).
	- distribution in the form of a function
	- or a fixed number
	```
	cd macros
	root -b -q 'Fun4Sim_pileup.C(100)'
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
