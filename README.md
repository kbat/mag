# GAM - Genetic Algorithm with Matrices

The code in this repository implements methodology described in
[https://doi.org/10.1002/mp.15339](https://doi.org/10.1002/mp.15339)
and extended to support the transport of multiple particle types.

Currently, four particle types are transported: e, n, p, and |.

There are two executables: solver (`gam-solve`) and optimiser (`gam`).

They need these [ROOT](https://root.cern) data files with
pre-calculated transport matrices:

```scp clu0-fe-0:~konbat/mat.tar.bz2 .```

## Solver
Run 20 layers of Tungsten followed by 4 layers of polyethylene with incident 3 GeV electrons:

```gam-solve -layers 20 W 4 Poly -sdef e 3e3 1.0```

Each layer is 1 cm thick.

The output shows the layer configuration and dose rates for each transported particle type:
```
24 layers: W W W W W W W W W W W W W W W W W W W W Poly Poly Poly Poly
Dose rates: e: 7.50763e-06  n: 0.00752361   p: 0.000113037  |: 2.62634e-25  total: 0.00764416
```


## Optimiser
Optimise materials of 10 layers to minimise the figure of merit:

```./gam -nlayers 10 -ngen 2```

With 10 layers and 5 known materials, the code runs approximately 180 configurations for each generation,
with the exact number depending on the available number of cores.

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
  [Optimiser::getFitness](https://github.com/kbat/gam/blob/master/src/Optimiser.cxx). Currently,
  it's hard-coded as a linear combination of dose, mass and
  complexity.
* **Dose** is equivalent dose beyond the layers. The flux-to-dose
 conversion factors are defined for neutrons in
 [Solver::getNeutronFTD](https://github.com/kbat/gam/blob/master/src/Solver.cxx)
 and in the other similar methods for the other particles.
* **Mass** is the sum of densities of all layers. This is probably
  misleading, but I still call it *mass* because this value is
  proportional to the total mass (since all layers have the same
  thickness of 1 cm).  Actually, since our problem is 2D, the layer is
  infinite in the directions perpendicular to the beam, therefore its
  mass is infinite.
* **Complexity** is the number of material changes in the material list.
* **Materials** list of material numbers for each layer. Material numbers can be printed with ``gam -mat``.
