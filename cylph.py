#! /usr/bin/env python

import os
import sys
import math
import argparse
import textwrap
import numpy as np
from contextlib import redirect_stdout

class Base:
    width=80     # textwrap width
    indent=" "*7 # textwrap indent

class Material(Base):
    def __init__(self, n, name, den):
        self.n = n
        self.name = name
        self.den = den
        self.comp = []
        self.mt = ""
        self.mx = {}
        self.lib = "hlib=.70h pnlib=70u"

    def addComponent(self, comp):
        self.comp.append(comp)

    def setMT(self, mt):
        self.mt = mt

    def setMX(self, particle, line):
        assert len(particle) == 1, "Particle identifier must be a char"
        self.mx[particle] = line

    def setLib(self, lib):
        self.lib = lib # override default value

    def Print(self):
        print(f"c {self.name}")
        comp = " "
        comp = comp.join(self.comp)
        print(textwrap.fill(f"m{self.n}  {comp}", width=self.width, subsequent_indent=self.indent))
        if self.lib:
            print(textwrap.fill(self.lib, initial_indent=self.indent))
            print("why 70u but not .70u?",file=sys.stderr)
        if self.mt:
            print(f"mt{self.n} {self.mt}")
        if self.mx:
            for p,line in self.mx.items():
                print(f"mx{self.n}:{p} {line}")


class Simulation(Base):
    def __init__(self, matname, version):
        self.matname = matname
        self.version = version
        self.particles = ('n', 'p', 'e', '|')

        self.path = os.path.join(self.matname, self.version)

        self.materials = {}

        m1 = Material(1, "Water", 1.0)
        m1.addComponent("1001.70c 0.0668456 1002.70c 1.003e-05 8016.70c 0.0334278")
        m1.setMT("lwtr.01t")
        m1.setMX("p", "1002 3j")
        self.materials[m1.name] = m1

        m49 = Material(49, "Concrete", 2.33578)
        m49.addComponent("1001.70c 0.00776555 1002.70c 1.16501e-06 8016.70c 0.0438499")
        m49.addComponent("11023.70c 0.00104778 12000.60c 0.000148662 13027.70c 0.00238815")
        m49.addComponent("14028.70c 0.0158026 16032.70c 5.63433e-05 19000.60c 0.000693104")
        m49.addComponent("20000.60c 0.00291501 26054.70c 1.84504e-05 26056.70c")
        m49.addComponent("0.000286826 26057.70c 6.5671e-06 26058.70c 8.75613e-07")
        m49.setMT("lwtr.01t")
        m49.setMX("p", "1002 3j 12026 3j 20040 20040 4j")
        self.materials[m49.name] = m49

        self.mat = self.materials[matname]


    def setEnergy(self, emin, emax, n):
        """ Set energy binning

        emin: minimum energy [MeV]
        emax: maximum energy [MeV]
        n: number of log equal bins between emin and emax
        i.e. specifying emin=1e-6, emax=3001, n=99 corresponds to the MCNP card
        e0 1e-6 99i 3001 and defines 101 energy bins between 0 and 3001 MeV
        """
        assert emin<emax, "emin >= emax"
        self.emin = emin
        self.emax = emax
        self.nbinse = n
        logemin = math.log10(self.emin)
        logemax = math.log10(self.emax)
        self.ebins  = np.linspace(logemin, logemax, num=n+2)
        self.ebins  = [0.0] + [pow(10, e) for e in self.ebins]

    def printEnergyTally(self):
        print(f"e0 {self.emin} {self.nbinse}log {self.emax}")

    def setNCosine(self, n):
        """ Set number of cosine bins

        n: number of lin equal bins between -PI to PI
        """
        assert n % 2 == 0, "Number of bins must be even"
        self.cbins = np.linspace(-math.pi, 0.0, num=n+1)
        self.cbins = list(map(math.cos, self.cbins))

    def printCosineTally(self):
        print("c0   ",textwrap.fill(" ".join(map("{:.5f}".format, self.cbins[1:])),
                                  width=self.width,
                                  subsequent_indent=self.indent))

    def printGeometry(self, mat, par, erg, dir, thick=1):
        print("GAM")
        print(f"c Incident particle: {par}")
        print("c Energy: %g MeV [average]" % ((erg[0] + erg[1])/2.0))
        print("c Direction cosine: %g [average]" % ((dir[0]+dir[1])/2.0))
        print("1 0 7")
        print("2 0 -7 -1")
        print("3 0  -7 2")
        print(f"4 {mat.n} {-mat.den} -7 1 -2 $ {mat.name}")
        print("")
        print("1 py 0.0")
        print(f"2 py {thick}")
        print("7 so 1e5")
        print("")

    def printSDEF(self, par, erg, dir):
        print(f"sdef par={par} erg=d1 pos=0 1e-4 0 dir=d2 vec=0 1 0")
        print(f"si1 {erg[0]:.5e} {erg[1]:.5e}")
        print(f"sp1 -21 0")
        print(f"si2 {dir[0]:.5f} {dir[1]:.5f}")
        print(f"sp2 -21 0")

    def printTallies(self):
        print("fc1 neutrons")
        print("f1:n 1 2")
        print("fc11 photons")
        print("f11:p 1 2")
        print("fc21 electrons")
        print("f21:e 1 2")
        print("fc31 muons")
        print("f31:| 1 2")

    def printPhysics(self):
        print("mode ", " ".join(self.particles))
        print("fcl:n,p 3j 1")
        print("imp:%s 0 1 2r" % ",".join(self.particles))
        print(f"phys:n {self.emax}")
        print("cut:n j 0.0")
        print(f"phys:p {self.emax} 2j 1")
        print("prdmp 2j 1")
        print("stop ctme 120")

    def printInp(self, inp, mat, par, erg, dir, thick=1):
        with open(inp, "w") as f:
            with redirect_stdout(f):
                self.printGeometry(mat, par, erg, dir, thick)
                mat.Print()
                self.printSDEF(par, erg, dir)
                self.printEnergyTally()
                self.printCosineTally()
                self.printTallies()
                self.printPhysics()

    def Print(self):
        # n is total number of cases
        # needed to format the folder name number
        nebins = len(self.ebins)
        ncbins = len(self.cbins)
        n = len(self.particles) * nebins * ncbins
        n = max(3, math.ceil(math.log10(n)))
        case=1
        ncbins = int(ncbins/2) # run towards forward hemisphere only
        for par in self.particles:
            for ebin in range(nebins-1):
                erg = (self.ebins[ebin], self.ebins[ebin+1])
                for cbin in range(ncbins):
                    dir = (self.cbins[ncbins+cbin], self.cbins[ncbins+cbin+1])
                    path = os.path.join(self.path, f"case%.{n}d" % case)
                    os.makedirs(path)
                    self.printInp(os.path.join(path, "inp"), self.mat, par, erg, dir)
                    case += 1






def main():
    parser = argparse.ArgumentParser(description=main.__doc__,
                                     epilog="Homepage: https://github.com/kbat/gam")
    parser.add_argument('mat',   type=str, help='Material name. A folder with the same name will be created for the generated input files. The material with this name must exist in the material database defined in the Simulation class constructor.')
    parser.add_argument('version',   type=str, help='Simulation version identifier. A folder with this name will be created under the material folder.')
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    run = Simulation(args.mat, args.version)
    run.setEnergy(1e-6, 3001, 99)
    run.setNCosine(18) # TODO: this number should be half of total (forward hemisphere only) - no need to check if even
    # run.setEnergy(1e-6, 3001, 0)
    # run.setNCosine(4)
    run.Print()


if __name__=="__main__":
    sys.exit(main())
