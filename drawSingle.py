#!/usr/bin/env python

import sys, argparse, os
import ROOT
from data import Data
from mctools.common.CombLayer import getPar as getParCL

ROOT.PyConfig.IgnoreCommandLineOptions = True

def main():
    """ Draws Transmitted/reflected histograms from a single data file """

    parser = argparse.ArgumentParser(description=main.__doc__,
                                         epilog="Homepage: https://github.com/kbat/mc-tools")
    parser.add_argument('mctal', type=str, help='mctal.root file name')
    parser.add_argument("-inp",  type=str, help='MCNP input file names (assumed to be in the same folder as mctal.root)', default="inp")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    mctal = args.mctal
    inp = mctal.replace(os.path.basename(args.mctal), args.inp)
    print(args.mctal, inp)

    par = getParCL(inp, "Particle:") # incident particle
    E0 = getParCL(inp, "Energy:")
    mu0 = getParCL(inp, "Direction cosine:", 3)

    ROOT.gStyle.SetOptStat(0)

    c1 = ROOT.TCanvas("c1", args.mctal)
    c1.Divide(3,2)

    n = Data(args.mctal, E0, mu0, 1)
    c1.cd(1)
    n.histT.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(4)
    n.histR.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    e = Data(args.mctal, E0, mu0, 11)
    c1.cd(2)
    e.histT.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(5)
    e.histR.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    p = Data(args.mctal, E0, mu0, 21)
    c1.cd(3)
    p.histT.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(6)
    p.histR.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    print(p.R)
    print(len(p.R))

#    p.PrintEbins(p.histT)

    input()

if __name__ == "__main__":
    sys.exit(main())
