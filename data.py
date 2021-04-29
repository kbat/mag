#!/usr/bin/env python

import ROOT
from math import log10

class Data:
    """Convert THnSparse into raws of Reflected and Transported
    histograms. The class assumes that the tally 'n' has (1) two
    surface bins: first is the backward surface (reflected), and
    second is the forward surface (transmitted); (2) a number of
    cosine bins spanning from -1 to 1 with a bin boundary at 0, the
    bins between -1 and 0 are associated to reflected part, the bins
    above 0 are associated to the transmitted part; (3) a number of
    energy bins.
    """
    def __init__(self,fname, n):
        f = ROOT.TFile(fname)
        tally = f.Get("f%d" % n)
#        tally.Print("a")

        # tmp!!!
#        tally.GetAxis(6).SetRangeUser(0, 5.0)

        # Reflected
        tally.GetAxis(0).SetRange(1,1); # surf 1
        tally.GetAxis(5).SetRangeUser(-1.0, 0.0) # back
        self.histR = tally.Projection(5,6)
        self.histR.SetNameTitle("R", "Reflected %s;Energy [MeV];#mu" % tally.GetTitle())
        self.histR.SetDirectory(0)
        self.R = self.buildRaw(self.histR)

        # Transmitted
        tally.GetAxis(0).SetRange(2,2); # surf 2
        tally.GetAxis(5).SetRangeUser(0.0, 1.0) # forward
        self.histT = tally.Projection(5,6)
        self.histT.SetNameTitle("T", "Transmitted %s;Energy [MeV];#mu" % tally.GetTitle())
        self.histT.SetDirectory(0)
        self.T = self.buildRaw(self.histT)

        f.Close()

    def buildRaw(self, hist):
        """ Build a raw from hist """
        nx = hist.GetNbinsX()
        ny = hist.GetNbinsY()
        raw = []
        for j in range(1,ny+1):
            for i in range(1,nx+1):
                raw.append(hist.GetBinContent(i,j))
        return raw

    def PrintEbins(self, M):
        """ Print mid energy bins of the M matrix """
        a = M.GetXaxis()
        nb = a.GetNbins()
        print("number of x-axis bins: ", nb, a.GetBinLowEdge(1))

        for i in range(nb):
            emin = a.GetBinLowEdge(i+1)
            if emin<0:
                emin = 0.0001
            emax = a.GetBinUpEdge(i+1)
            val = (emin+emax)/2.0 # linear
#            val = pow(10,(log10(emin)+log10(emax))/2.0) # log
            print(val, end=" ")
        print("")
