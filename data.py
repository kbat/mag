#!/usr/bin/env python

import ROOT
from math import log10

class Data:
    """ Convert THnSparse into individual
        Reflected and Transported histograms
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
        self.R = tally.Projection(5,6)
        self.R.SetNameTitle("R", "Reflected %s;Energy [MeV];#mu" % tally.GetTitle())
        self.R.SetDirectory(0)

        # Transmitted
        tally.GetAxis(0).SetRange(2,2); # surf 2
        tally.GetAxis(5).SetRangeUser(0.0, 1.0) # forward
        self.T = tally.Projection(5,6)
        self.T.SetNameTitle("T", "Transmitted %s;Energy [MeV];#mu" % tally.GetTitle())
        self.T.SetDirectory(0)

        f.Close()

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
