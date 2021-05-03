#!/usr/bin/env python

import sys, argparse, os
from data import Data, Matrix, Source, readData
from copy import copy

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

def main():
    """ Solve matrix transport equation
    """

    ROOT.gStyle.SetPalette(ROOT.kRust)

    parser = argparse.ArgumentParser(description=main.__doc__,
                                     epilog="Homepage: https://github.com/kbat/mc-tools")
    parser.add_argument('dir',   type=str, help='folder with case*/mctal.root files')
    parser.add_argument("-mctal",type=str, help='mctal.root file names', default="mctal.root")
    parser.add_argument("-inp",  type=str, help='MCNP input file names (assumed to be in the same folder as mctal.root)', default="inp")
    parser.add_argument('-v', '--verbose', action='store_true', default=False, dest='verbose', help='explain what is being done')

    args = parser.parse_args()

    assert os.path.isdir(args.dir), "%s is not a folder or does not exist" % args.dir

    d0 = Data(os.path.join(args.dir,"case020","mctal.root"), 3.69604, 0.15, 1)
    h0 = d0.histT
    h0.Reset()

    print(d0)

    h0.Fill(707.732, 0.05)
    h0.SetTitle("Source TH2")

    s = Source(h0)

    c1 = ROOT.TCanvas()
    c1.Divide(2,2)

    c1.cd(1)
    h0.Draw("col,text")
    ROOT.gPad.SetLogx()

    c1.cd(2)
    hSource = ROOT.TH1D(s.S)
    hSource.SetTitle("Source row")
    hSource.Draw()

    c1.cd(3)
    res = copy(s) # result

    m = readData('n', os.path.join(args.dir, "case*", args.mctal)) # matrix
#    print(sorted(m.energies), sorted(m.directions))

#    res.S.Print()

    res.S *= m.T.T()

#    res.S.Print()

    hResult = ROOT.TH1D(res.S)
    hResult.SetTitle("Result row")
    hResult.Draw()
    ROOT.gPad.SetLogy()

    c1.cd(4)
    h = res.fillHist()
    h.SetTitle("Penetrated")
    h1 = h.ProjectionX()
    h1.Draw()
    ROOT.gPad.SetLogx()
    ROOT.gPad.SetLogy()

    # compare with original data:
    fOrig = ROOT.TFile("water/case012/mctal.root")
    f1 = fOrig.Get("f1")
    f1.GetAxis(0).SetRange(2,2)
    f1.GetAxis(5).SetRange(11,20)
    hOrig = f1.Projection(6)

    f1.Print("a")

    n = h1.GetNbinsX()
    print("bins: ", n)
    for b in range(1,n+1):
        print(b, h1.GetBinContent(b), hOrig.GetBinContent(b), "\t", h1.GetBinContent(b)-hOrig.GetBinContent(b))


    c1.Update()

    input()


if __name__ == "__main__":
    sys.exit(main())
