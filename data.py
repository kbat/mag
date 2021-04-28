#!/usr/bin/env python

import ROOT

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
