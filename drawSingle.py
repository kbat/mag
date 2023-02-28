#!/usr/bin/env python

import sys, argparse, os
import ROOT
from data import Data
from mctools.common.CombLayer import getPar as getParCL
from mctools.common import ErrorHist

ROOT.PyConfig.IgnoreCommandLineOptions = True

def fixPad():
    # ROOT.gPad.Modified()
    ROOT.gPad.SetLogx()
    ROOT.gPad.Update()

def draw(cnv, particles, drawErrors=False):
    N = len(particles)
    cnv.Divide(N, 2)

    pads = range(1, N+1)
    for p, pad in zip(particles, pads):
        if drawErrors:
            # print(pad, p.histT.GetTitle())
            cnv.cd(pad)
            ErrorHist(p.histT).DrawClone("colz")
            fixPad()

            cnv.cd(pad+N)
            # print(pad+N, p.histR.GetTitle())
            ErrorHist(p.histR).DrawClone("colz")
            fixPad()
        else:
            cnv.cd(pad)
            p.histT.Draw("colz")
            fixPad()

            cnv.cd(pad+N)
            p.histR.Draw("colz")
            fixPad()

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

    par = getParCL(inp, "Incident particle:") # incident particle
    E0  = getParCL(inp, "Energy:")
    mu0 = getParCL(inp, "Direction cosine:", 3)

    ROOT.gStyle.SetOptStat(0)


    n = Data(args.mctal, E0, mu0, 1)
    e = Data(args.mctal, E0, mu0, 11)
    p = Data(args.mctal, E0, mu0, 21)
    m = Data(args.mctal, E0, mu0, 31)
    particles = [n,e,p,m]

    # canvas with values
    c1 = ROOT.TCanvas("c1", args.mctal)
    draw(c1, particles, False)
    c1.Print("a.pdf(")

    # canvas with relative errors
    c2 = ROOT.TCanvas("c2", args.mctal)
    draw(c2, particles, True)
    c2.Print("a.pdf)")

    input()

if __name__ == "__main__":
    sys.exit(main())
