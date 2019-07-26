# EvoMotion
__author__ : Samuel Berrien

## Description
The final goal of this project is to implement CMA-ES or any kind of optimisation algorithm in order to teach creature to walk, run etc.

__Note__ : In a first time, only CMA-ES will be implemented and tested with [coco](https://github.com/numbbo/coco) benchmark tools. Then the creature walk teaching will be implemented.

## Requirements
This project requires LAPACK, BLAS, Boost and Armadillo libraries.
### Ubuntu
LAPACK, BLAS and Boost :
```bash
$ sudo apt-get install liblapack-dev
$ sudo apt-get install libblas-dev
$ sudo apt-get install libboost-dev
```

Armadillo :
download latest version on offical [website](http://arma.sourceforge.net/download.html)
```bash
$ # change directory to extracted armadillo dir
$ cd armadillo
$ # then install
$ cmake .
$ make
$ sudo make install
```

## Installation
First clone this repository :
```bash
$ git clone https://github.com/Ipsedo/EvoMotion.git
$ cd EvoMotion
```

Then install coco benchmark platform :
```bash
$ cd EvoMotion
$ ./install_coco.sh
```

Finally you are able to build the source
```bash
$ cd EvoMotion
$ ./build.sh
```


