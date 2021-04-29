#!/usr/bin/env python

import sys, argparse, glob
import ROOT
from data import Data, Matrix
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
    parser.add_argument("-cylph",  type=str, help='pstudy input file name', default="cylph")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    incidentParticle = "e"
    e = Matrix(incidentParticle)
    for mctal in glob.glob(args.dir+"/case*/"+args.mctal):
        inp = mctal.replace(args.mctal, args.inp)
        print(mctal)
        par = getParCL(inp, "Particle:")
        # if par == incidentParticle:
        e.append(mctal)

    e.run()

    he = ROOT.TH2D(e.T)
    he.SetTitle(e.particle)
    he.Draw("col")
    input()

if __name__ == "__main__":
    sys.exit(main())
