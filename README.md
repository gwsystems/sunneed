[![Logo](res/logo.svg)](res/logo.svg)

[![Build Status](https://dev.azure.com/gschock/suneed/_apis/build/status/grahamschock.power_mgmt_infra?branchName=master)](https://dev.azure.com/gschock/suneed/_build/latest?definitionId=2&branchName=master)

# power_mgmt_infra

We are creating a framework for tracking and managing the distribution of power consumption of individual processes in
multi-tenant computing environments. In systems with uncertain reserves of power available, such as a computer powered
by a solar battery, it is impossible to guarantee unlimited power to each tenant. The basis of this project is to make a
toolset for tenants to run **power-constrained code** on the system, limited by a power budget described by `sunneed`.

# Installation

Below is instructions for getting a basic instance of `sunneed` running on a Linux host. Package names are intentionally
left ambiguous due to the variety of naming schemes for packages across distros; see the `misc/install_dependencies`
file for package names on Ubuntu 18.04.

If you are running Ubuntu 18.04 (and likely other versions of Ubuntu and Debian), you may run`misc/install_dependencies` 
(with root privileges) and then skip to [Dependencies of Dependencies](#dependencies-of-dependencies).

## Getting the dependencies

`sunneed` uses protobufs for I2C. Specifically, it uses the `protobufs-c` library. Both the `protoc-c` compiler and the
`libprotobuf-c` headers and libraries must be installed.

### Dependencies of dependencies

We also need to manually compile some dependencies. Run `git submodule update --init --recursive`. This will download
the code for NNG and libbq27441, which must be compiled manually on common distros.

For building NNG, you will need CMake > 3.13 and Ninja. These are available as packages on common distros.

For building libbq27441, you will need the headers for the Linux I2C library. This comes under different names depending
on the distro; you want whichever package gives you the file `/usr/include/linux/i2c-dev.h`.

## Compilation

Once you have all the dependencies in place, we can begin compilation. These steps assume you cloned this repo into
`~/src`

Change to the base directory:

```
~/src/power_mgmt_infra$ cd src
```

Compile the dependencies:

```
~/src/power_mgmt_infra/src$ make
```

Change to the `sunneed` base directory:

```
~/src/power_mgmt_infra/src$ cd sunneed
```

Compile `sunneed`:

```
~/src/power_mgmt_infra/src/sunneed$ make
```

After all this, there should be a file `build/sunneed` in the `sunneed` directory. This is the core `sunneed` binary.

<!-- vim: set tw=120: -->
