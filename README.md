# gam
Get and untar the ROOT files with transport matrices: ```scp clu0-fe-0:~konbat/mat.tar.bz2 .```

## Solver
Run 20 layers of Tungsten followed by 4 layers of polyethylene with incident 3 GeV electrons:

```gam-solve -layers 20 W 4 Poly -sdef e 3e3 1.0```

Each layer is 1 cm thick


## Optimiser
Optimise materials of 10 layers to minimise the figure of merit:

```./gam -nlayers 10 -ngen 2```

The code runs approximately 180 configurations for each generation,
with the exact number depending on the available number of cores.

The output for each generation consists of the runtime line followed
by a list of five best solutions, e.g.

```
Generation: 1
2.17279 sec
Fitness         Dose            Mass    Complexity Materials
0.185034        0.185034        194.13  1          38 38 38 38 38 38 38 38 38 38
0.228578        0.228578        185.901 2          38 38 38 38 38 38 38 38 38 23
0.251875        0.251875        185.901 2          23 38 38 38 38 38 38 38 38 38
0.255914        0.255914        182.684 2          38 38 38 38 38 38 38 38 38 3
0.282889        0.282889        177.223 2          38 38 38 38 38 38 38 38 38 47
```

* The **fitness function** is the figure of merit for the optimisation
  algorithm defined in
  [Optimiser::getFitness](https://github.com/kbat/gam/blob/master/src/Optimiser.cxx). Currently,
  it's a linear combination of dose, mass and complexity.
* **Dose** is the equivalent dose beyond the layers. The flux-to-dose
 conversion factors are defined for neutrons in
 [Solver::getNeutronFTD](https://github.com/kbat/gam/blob/master/src/Solver.cxx)
 and in the other similar methods for the other particles.  *
