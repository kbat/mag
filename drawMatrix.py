#!/usr/bin/env python

import sys, argparse, glob
import ROOT
from data import Data
import pandas as pd

ROOT.PyConfig.IgnoreCommandLineOptions = True

def getN(args):
    """ Get square matrix order
    """
    for mctal in glob.glob(args.dir+"/case*/"+args.mctal):
        e = Data(mctal, 11)
        return len(e.T)


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

    N = getN(args)

    vecT = []
    vecR = []

    for mctal in glob.glob(args.dir+"/case*/"+args.mctal):
        e = Data(mctal, 11)
        vecT.append(e.T)
        vecR.append(e.R)

    T = pd.DataFrame(vecT)
    R = pd.DataFrame(vecR)
    print(T)



    # c1 = ROOT.TCanvas("c1", args.fname)

    # input()

if __name__ == "__main__":
    sys.exit(main())
