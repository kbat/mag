# MAG - Markov chains and Genetic algorithms

The code in this repository is designed to conduct 1D particle
transport calculations using Markov chains and optimise geometry
layout using genetic algorithms.

## Installation

### Dependencies
* Linux (should work on MAC, but never tested on Windows yet)
* C++ compiler with a minimum standard of C++17
* [ROOT](https://root.cern)
* [libtbb-dev](https://en.wikipedia.org/wiki/Threading_Building_Blocks)
* [libboost-program-options](https://www.boost.org/doc/libs/1_63_0/doc/html/program_options.html)

The last two packages are standard and available in any modern Linux
flavours. However, if ROOT is not available for your specific
distribution, you can easily install it by following the instructions
provided on [this page](https://root.cern/install).

### Compile
```
git checkout https://github.com/kbat/mag.git mag
cd mag
mkdir build
cd build
cmake ..
make -j$(nproc)
```

The compiled code consists of two separate executables: solver
(`mag-solve`) which is intended for particle transport calculations,
and optimiser (`mag-optimise`) which is specifically designed for
geometry optimisation tasks.

## Solver

List supported materials:
```
mag-solve -mat
```
The output shows material number, name and mass density:
```
Supported materials:
3  Stainless304 7.96703
11 Water        1
38 W            19.413
23 Lead         11.1837
47 B4C          2.50608
48 Poly         0.91
49 Concrete     2.33578
```

The material numbers can be chosen arbitrarily. Currently, they
currently correspond to the material numbers used in
[CombLayer](https://github.com/sansell/comblayer).


### Example

Run 20 layers of Tungsten followed by 4 layers of polyethylene with incident 3 GeV electrons:

```mag-solve -layers 20 W 4 Poly -sdef e 3e3 1.0 -o spectra.root```

Each layer is 1 cm thick.

The output shows the layer configuration and dose rates for each transported particle type:
```
24 layers: W W W W W W W W W W W W W W W W W W W W Poly Poly Poly Poly
Dose rates: e: 7.50763e-06  n: 0.00752361   p: 0.000113037  |: 2.62634e-25  total: 0.00764416
```

The particle designators adhere to the [MCNP](https://mcnp.lanl.gov) notation:

| ID | Description |
|---|---|
| e  | electron (e+ and e-) |
| n  | neutron  |
| p  | photon   |
| \| | muon (μ+ and μ-)     |
| h  | proton   |

## Optimiser

Optimise materials of 10 layers to minimise the fitness function:

```mag -nlayers 10 -ngen 2```

With 10 layers and 5 known materials, the code runs approximately 180 configurations for each generation,
with the exact number depending on the available number of cores on your machine.

The output for each generation consists of the runtime line followed
by a list of five best solutions, e.g.

```
Generation: 1
2.09588 sec
Fitness    Dose      Mass    Complexity Materials
7.88793    3.23793   9.1     1          48 48 48 48 48 48 48 48 48 48
8.91059    3.36255   10.6961 2          48 48 48 48 48 48 48 48 48 47
8.9364     3.38836   10.6961 2          47 48 48 48 48 48 48 48 48 48
9.84813    3.50205   12.2922 2          48 48 48 48 48 48 48 48 47 47
9.88769    3.54161   12.2922 2          47 47 48 48 48 48 48 48 48 48

Generation: 2
2.15738 sec
Fitness    Dose      Mass    Complexity Materials
7.88793    3.23793   9.1     1          48 48 48 48 48 48 48 48 48 48
8.91059    3.36255   10.6961 2          48 48 48 48 48 48 48 48 48 47
11.961     3.81885   15.4843 4          48 47 47 48 48 48 48 48 47 47
12.2642    3.82206   15.4843 7          48 48 48 47 47 48 47 48 47 48
13.2258    3.98564   17.0804 7          48 47 48 48 47 47 48 47 47 48
```

* The **fitness function** is the figure of merit for the optimisation
  algorithm defined in
  [Optimiser::getFitness](https://github.com/kbat/mag/blob/master/src/Optimiser.cxx). Currently,
  it's hard-coded as a linear combination of dose, mass and
  complexity.
* **Dose** is equivalent dose beyond the layers. The flux-to-dose
 conversion factors are defined for neutrons in
 [Solver::getNeutronFTD](https://github.com/kbat/mag/blob/master/src/Solver.cxx)
 and in the other similar methods for the other particles.
* **Mass** is the sum of densities of all layers. This is probably
  misleading, but I still call it *mass* because this value is
  proportional to the total mass (since all layers have the same
  thickness of 1 cm).  Actually, since our problem is 2D, the layer is
  infinite in the directions perpendicular to the beam, therefore its
  mass is infinite.
* **Complexity** is the number of material changes in the material list.
* **Materials** is the list of material numbers for each layer. Material numbers can be printed with ``mag-optimise -mat``.
