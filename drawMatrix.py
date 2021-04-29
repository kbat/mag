#!/usr/bin/env python

import sys, argparse, glob
import ROOT
from data import Data, Matrix
from array import array

ROOT.PyConfig.IgnoreCommandLineOptions = True

def main():
    """ Draws Transmitted/reflected histograms from a data file set
    """

    ROOT.gStyle.SetOptStat(0)

    parser = argparse.ArgumentParser(description=main.__doc__,
                                         epilog="Homepage: https://github.com/kbat/mc-tools")
    parser.add_argument('dir',   type=str, help='folder with case*/mctal.root files')
    parser.add_argument("-mctal",type=str, help='mctal ROOT file names', default="mctal.root")
    parser.add_argument("-inp",  type=str, help='MCNP input file names', default="inp")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    e = Matrix("n")
    for mctal in glob.glob(args.dir+"/case*/"+args.mctal):
        e.append(mctal)

    e.run()

    he = ROOT.TH2D(e.T)
    he.SetTitle(e.particle)
    he.Draw("col")
    input()

if __name__ == "__main__":
    sys.exit(main())
