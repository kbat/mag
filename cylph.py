#! /usr/bin/env python

import sys
import argparse
import textwrap

class Material:
    def __init__(self, n, name, den):
        self.n = n
        self.name = name
        self.den = den
        self.comp = []
        self.mt = ""
        self.mx = {}
        self.lib = "hlib=.70h pnlib=70u"
        self.width=80     # textwrap width
        self.indent=" "*7 # textwrap indent

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
        comp = " "
        comp = comp.join(self.comp)
        print(textwrap.fill(f"m{self.n}  {comp}", width=self.width, subsequent_indent=self.indent))
        if self.lib:
            print(textwrap.fill(self.lib, initial_indent=self.indent))
            print("why 70u but not .70u?")
        if self.mt:
            print(f"mt{self.n} {self.mt}")
        if self.mx:
            for p,line in self.mx.items():
                print(f"mx:{p} {line}")



def printGeometry(f, mat, par, erg, dir, thick=1):
    print("GAM")
    print(f"c Incident particle: {par}")
    print(f"c Energy: {erg}")
    print(f"c Direction cosine: {dir}")
    print("1 0 7")
    print("2 0 -7 -1")
    print("3 0  7 -2")
    print(f"4 {mat.n} {-mat.den} -7 1 -2 $ {mat.name}")
    print("")
    print("1 py 0.0")
    print(f"2 py {thick}")
    print("7 so 1e5")
    print("")

def printSDEF(f, par, erg, dir):
    print(f"sdef par={par} erg={erg} pos=0 -0.1 0 dir={dir} vec=0 1 0")


def main():
    parser = argparse.ArgumentParser(description=main.__doc__,
                                     epilog="Homepage: https://github.com/kbat/gam")
    parser.add_argument('dir',   type=str, help='folder with case*/mctal.root files')
    parser.add_argument("-mctal",type=str, help='mctal.root file names', default="mctal.root")
    parser.add_argument("-inp",  type=str, help='MCNP input file names (assumed to be in the same folder as mctal.root)', default="inp")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    m1 = Material(1, "Water", 1.0)
    m1.addComponent("1001.70c 0.0668456 1002.70c 1.003e-05 8016.70c 0.0334278")
    m1.setMT("lwtr.01t")
    m1.setMX("p", "1002 3j")

    m49 = Material(49, "Cocrete", 2.33578)
    m49.addComponent("1001.70c 0.00776555 1002.70c 1.16501e-06 8016.70c 0.0438499")
    m49.addComponent("11023.70c 0.00104778 12000.60c 0.000148662 13027.70c 0.00238815")
    m49.addComponent("14028.70c 0.0158026 16032.70c 5.63433e-05 19000.60c 0.000693104")
    m49.addComponent("20000.60c 0.00291501 26054.70c 1.84504e-05 26056.70c")
    m49.addComponent("0.000286826 26057.70c 6.5671e-06 26058.70c 8.75613e-07")
    m49.setMT("lwtr.01t")
    m49.setMX("p", "1002 3j 12026 3j 20040 20040 4j")


    mat = m49

    par = "p"
    erg = 3000
    dir = 1.0

    printGeometry(1, mat, par, erg, dir)
    mat.Print()
    printSDEF(1, par, erg, dir)



if __name__=="__main__":
    sys.exit(main())
