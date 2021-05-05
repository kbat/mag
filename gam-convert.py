#!/usr/bin/env python

import sys, argparse, os
from data import Data, Matrix, Source, readData
from copy import copy

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

def main():
    """Convert mctal.root files into transport matrices

    """

#    ROOT.gStyle.SetPalette(ROOT.kRust)
    ROOT.gStyle.SetOptStat(False)

    parser = argparse.ArgumentParser(description=main.__doc__,
                                     epilog="Homepage: https://github.com/kbat/mc-tools")
    parser.add_argument('dir',   type=str, help='folder with case*/mctal.root files')
    parser.add_argument("-mctal",type=str, help='mctal.root file names', default="mctal.root")
    parser.add_argument("-inp",  type=str, help='MCNP input file names (assumed to be in the same folder as mctal.root)', default="inp")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    assert os.path.isdir(args.dir), "%s is not a folder or does not exist" % args.dir

    E0 = 707.732 # dummy
    mu0 = 0.05   # dummy
    fname = os.path.join(args.dir,"case001",args.mctal)

    d0 = Data(fname, E0, mu0, 1)
    h0 = d0.histT
    h0.Reset()
    h0.SetName("h0")

    print(d0)

    m = readData('n', os.path.join(args.dir, "case*", args.mctal)) # matrix

    hT = ROOT.TH2D(m.T)
    hT.SetNameTitle("T", m.particle)

    hR = ROOT.TH2D(m.R)
    hR.SetNameTitle("R", m.particle)

    fout = ROOT.TFile("gam.root", "recreate")
    hT.Write()
    hR.Write()
    h0.Write()
    fout.Close()


if __name__ == "__main__":
    sys.exit(main())
