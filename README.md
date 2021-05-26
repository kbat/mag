# gam
Get and untar the ROOT files with transport matrices: ```scp clu0-fe-0:~konbat/mat.tar.bz2 .```

Run 20 layers of Tungsten followed by 4 layers of polyethylene:
```gam-solve 20 Tungsten 4 Poly```

Optimise 150 layers of different materials (no command line options available yet, see src/gam.cxx):
```./gam```
It will take some time - better run it on a cluster or reduce number of layers (to e.g. 10).
