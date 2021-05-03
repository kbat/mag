#!/usr/bin/env python

import sys, argparse, glob, os
import ROOT
from data import Data, Matrix, Source, readData
from array import array
from mctools.common.CombLayer import getPar as getParCL

ROOT.PyConfig.IgnoreCommandLineOptions = True

def main():
    """ Draws Transmitted/reflected histograms from a data file set
    """

    ROOT.gStyle.SetOptStat(0)

    parser = argparse.ArgumentParser(description=main.__doc__,
                                         epilog="Homepage: https://github.com/kbat/mc-tools")
    parser.add_argument('dir',   type=str, help='folder with case*/mctal.root files')
    parser.add_argument("-mctal",type=str, help='mctal.root file names', default="mctal.root")
    parser.add_argument("-inp",  type=str, help='MCNP input file names (assumed to be in the same folder as mctal.root)', default="inp")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    m = readData('n', os.path.join(args.dir, "case*", args.mctal))

    h = ROOT.TH2D(m.T)
    h.SetTitle(m.particle)
    h.Draw("colz")


    # d0 = Data(args.dir+"/case020/mctal.root", 3.69604, 0.15, 1)
    # h0 = d0.histT
    # h0.Draw("col")

    # s = Source(h0)

    # F = s.Mult(

    input()

if __name__ == "__main__":
    sys.exit(main())
