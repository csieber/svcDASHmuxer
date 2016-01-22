# svcDASHmuxer

A muxer for raw scalable h264 Annex.G (SVC) bitstreams which where segmented into different representations.

## Requirements

 - premake4
 - g++
 - libboost-system-dev 
 
## Compile

Use premake4 to create the make files:

```bash
git clone git@github.com:csieber/svcDASHmuxer.git
cd svcDASHmuxer/
premake4 gmake
```

Build the application:

```bash
cd build/
make
```

## Usage

```
Usage: svcDASHmuxer [Output file][Base Layer] [Base Layer + 1] [...]
```

Check the vagrant [here](vagrant/README.md) for an example how to use the muxer.

## Citing

Please cite the following publication if you use this application:

```
@inproceedings{sieber2013implementation,
  title={Implementation and User-centric Comparison of a Novel Adaptation Logic for DASH with SVC},
  author={Sieber, Christian and Ho{\ss}feld, Tobias and Zinner, Thomas and Tran-Gia, Phuoc and Timmerer, Christian},
  booktitle={2013 IFIP/IEEE International Symposium on Integrated Network Management (IM 2013)},
  pages={1318--1323},
  year={2013},
  organization={IEEE}
}
```