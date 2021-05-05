#!/usr/bin/env python

import sys, argparse, os
from data import Data, Matrix, Source, readData
from copy import copy

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

def getReflectionsT(T, Tn, R, Rn, order=1):
    """Calculate reflection term of the given order for the T matrix """

    assert order == 1, "Other orders not supported yet"

    m = copy(Tn)
    m *= R
    m *= Rn
    m *= T

    return m

def getReflectionsR(T, Tn, R, Rn, order=1):
    """Calculate reflection term of the given order for the R matrix """

    assert order == 1, "Other orders not supported yet"

    m = copy(T)
    m *= Rn
    m *= T

    return m

def getNextT(T, Tn, R, Rn, order=1):
    """ Calculate next iteration T matrix """
    m = copy(Tn)
    m *= T
    m += getReflectionsT(T, Tn, R, Rn, order)
    return m

def getNextR(T, Tn, R, Rn, order=1):
    """ Calculate next iteration R matrix """
    m = copy(R)
    m += getReflectionsR(T, Tn, R, Rn, order)
    return m

def main():
    """ Solve matrix transport equation
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

    E0 = 707.732
    mu0 = 0.05
    fname = os.path.join(args.dir,"case012/50cm","mctal.root")

    d0 = Data(fname, E0, mu0, 1)
    h0 = d0.histT
    h0.Reset()

    print(d0)

    h0.Fill(E0, mu0) # case0992
    h0.SetTitle("Source TH2")

    s = Source(h0)

    c1 = ROOT.TCanvas()
    c1.Divide(3,2)

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

    T = m.T.T() # transpose the matrix
    R = m.R.T()

    T1 = copy(T)
    R1 = copy(R)
    #####################

    for i in range(50-1):
        T1 = getNextT(T, T1, R, R1)
        R1 = getNextR(T, T1, R, R1)

    res.S *= T1

#    res.S.Print()

    hResult = ROOT.TH1D(res.S)
    hResult.SetTitle("Result row")
    hResult.Draw()
    ROOT.gPad.SetLogy()

    c1.cd(4)
    h = res.fillHist()
    h.SetTitle("Penetrated")

    h.Draw("col")
    ROOT.gPad.SetLogx()

    c1.cd(5)

    h1x = h.ProjectionX()
    h1x.Draw()
    ROOT.gPad.SetLogx()
    ROOT.gPad.SetLogy()

    # compare with original data:
    fOrig = ROOT.TFile(fname)
#    fOrig = ROOT.TFile("water-E100/case0992/100cm/mctal.root")
#    fOrig = ROOT.TFile("water-E100/case0992/2cm/mctal.root")
    f1 = fOrig.Get("f1")
    f1.GetAxis(0).SetRange(2,2)
    f1.GetAxis(5).SetRangeUser(0.01,0.999)

    hOrigE = f1.Projection(6)
    hOrigE.SetLineColor(ROOT.kRed)
    hOrigE.Draw("hist,same,e")

    f1.Print("a")

    n = h1x.GetNbinsX()
    print("bins: ", n)
    for b in range(1,n+1):
        valOrig = hOrigE.GetBinContent(b)

        if valOrig > 0:
            print(b, h1x.GetBinContent(b), valOrig, "\t", h1x.GetBinContent(b)/valOrig)

    c1.cd(6)
    h1y = h.ProjectionY()
    h1y.Draw()

    hOrigMu = f1.Projection(5)
    hOrigMu.SetLineColor(ROOT.kRed)
    hOrigMu.Draw("hist,same,e")

    ROOT.gPad.SetLogy()




    c1.Update()

    input()


if __name__ == "__main__":
    sys.exit(main())
