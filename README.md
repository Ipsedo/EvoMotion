# EvoMotion
__author__ : Samuel Berrien

## Description
The final goal of this project is to implement CMA-ES or any kind of optimisation algorithm in order to teach creature to walk, run etc.

We will first try to reproduce the results of the article [Flexible Muscle-Based Locomotion for Bipedal Creatures](https://www.goatstream.com/research/papers/SA2013/SA2013.pdf)

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

Download the latest version of Armadillo on official [website](http://arma.sourceforge.net/download.html)
```bash
$ # Go to extracted armadillo directory
$ cd armadillo
$ # Then install
$ cmake .
$ make
$ sudo make install
```

## Installation
First clone this repository :
```bash
$ git clone https://github.com/Ipsedo/EvoMotion.git
```

Then install coco benchmark platform :
```bash
$ # Go to EvoMotion root directory
$ cd EvoMotion
$ # Clone and install COCO
$ ./install_coco.sh
```

Finally you are able to build the source
```bash
$ # Go to EvoMotion root directory
$ cd EvoMotion
$ # Build project with CMake and Makefile
$ ./build.sh
```


