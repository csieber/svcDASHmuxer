# Vagrant Example

## Requirements

 - Vagrant
 - VirtualBox
 
## Setup

Clone the repository, spin up the VM and ssh into the machine:

```bash
git clone git@github.com:csieber/svcDASHmuxer.git
cd svcDASHmuxer/

vagrant up
vagrant shh
```

Get the demuxed example files from the [svcDASHdemuxer](https://github.com/csieber/svcDASHdemuxer/tree/master/vagrant) project example and use svcDASHmuxer to put the segments back together:

```bash
cd /svcDASHmuxer/bin/

./svcDASHmuxer_d video5_0002.264 video5_00_0002.264 video5_01_0002.264
```

The output should look like this:

```
Opening & reading base layer file 'video5_00_0002.264'...DONE
Opening & reading 1nd enh. layer 'video5_0002.264'...DONE
Muxing the files together...DONE
```
