#!/usr/bin/env python

import sys, argparse, os
from data import Data, Matrix, Source, readData
from copy import copy

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

def getH0(args):
    """ Return empty histogram for energy / direction bins """

    E0 = 707.732 # dummy
    mu0 = 0.05   # dummy
    fname = os.path.join(args.dir,"case001",args.mctal)

    d0 = Data(fname, E0, mu0, 1)
    h0 = d0.histT
    h0.Reset()
    h0.SetNameTitle("sdef", "SDEF histogram [empty]")

    return h0

def main():
    """Convert mctal.root files into transport matrices for the given material

    """

#    ROOT.gStyle.SetPalette(ROOT.kRust)
    ROOT.gStyle.SetOptStat(False)

    parser = argparse.ArgumentParser(description=main.__doc__,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     epilog="Homepage: https://github.com/kbat/mc-tools")
    parser.add_argument('dir',   type=str, help='Folder name with case*/mctal.root files. It is assumed that the data in this folder is related to the same material but different incident energies and direction cosines.')
    parser.add_argument("-mctal",type=str, help='mctal.root file names', default="mctal.root")
    parser.add_argument("-inp",  type=str, help='MCNP input file names (assumed to be in the same folder as mctal.root).', default="inp")
    parser.add_argument("-par",type=str, help='Space-separated list of incident and scored particles.', default="n p e |")
    parser.add_argument("-o",  type=str, dest='output', help='Output file name. If not given, the folder name "dir"+.root will be used.', default=None)
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='Explain what is being done.')

    args = parser.parse_args()

    assert os.path.isdir(args.dir), "%s is not a folder or does not exist" % args.dir

    h0 = getH0(args)

    particles = args.par.split()

    m = readData(os.path.join(args.dir, "case*", args.mctal), particles) # dic of matrices

    T = []
    R = []

    for p0 in particles:
        for p in particles:
            if args.verbose:
                print("%s -> %s" % (p0, p))

            hT = ROOT.TH2D(m[p0][p].T)
            hT.SetNameTitle(p0+"T"+p, "T: %s #rightarrow %s" % (p0, p))
            T.append(hT)

            hR = ROOT.TH2D(m[p0][p].R)
            hR.SetNameTitle(p0+"R"+p, "R: %s #rightarrow %s" % (p0, p))
            R.append(hR)

    # save matrices
    fname = args.output
    if fname is None:
        fname = args.dir+".root"

    fout = ROOT.TFile(fname, "recreate")
    h0.Write()
    for t in T:
        t.Write()
    for r in R:
        r.Write()
    fout.Close()


if __name__ == "__main__":
    sys.exit(main())
