#!/usr/bin/env python

import ROOT
from sys import exit
from math import log10
from array import array

class Data:
    """Convert THnSparse into raws of Reflected and Transmitted
    matrices. The class assumes that the tally 'n' has (1) two
    surface bins: first is the backward surface (reflected), and
    second is the forward surface (transmitted); (2) a number of
    cosine bins spanning from -1 to 1 with a bin boundary at 0, the
    bins between -1 and 0 are associated to reflected part, the bins
    above 0 are associated to the transmitted part; (3) a number of
    energy bins.
    """
    def __init__(self,fname, E0, mu0, n):
        self.E0 = E0
        self.mu0 = mu0
        epsilon = 1e-3

        f = ROOT.TFile(fname)
        tally = f.Get("f%d" % n)

        # Reflected
        tally.GetAxis(0).SetRange(1,1); # surf 1
        tally.GetAxis(5).SetRangeUser(-1.0+epsilon, -epsilon) # back
        self.histR = tally.Projection(5,6)
        self.histR.SetNameTitle("R", "Reflected %s;Energy [MeV];#mu" % tally.GetTitle())
        self.histR.SetDirectory(0)
        self.R = self.buildRaw(self.histR)

        # Transmitted
        tally.GetAxis(0).SetRange(2,2); # surf 2
        tally.GetAxis(5).SetRangeUser(epsilon, 1.0-epsilon) # forward
        self.histT = tally.Projection(5,6)
        self.histT.SetNameTitle("T", "Transmitted %s;Energy [MeV];#mu" % tally.GetTitle())
        self.histT.SetDirectory(0)
        self.T = self.buildRaw(self.histT)

        f.Close()

        if len(self.R) != len(self.T):
            print("Error: R and T have different lengths", len(self.R), len(self.T))
            exit(1)

    def buildRaw(self, hist):
        """Build a _sorted_ raw from hist: inner loop is energy, outer loop is
        direction.

        """
        nx = hist.GetNbinsX()
        ny = hist.GetNbinsY()
        raw = []
        for j in range(1,ny+1):
            for i in range(1,nx+1):
                raw.append(hist.GetBinContent(i,j))
        return raw

    def printEbins(self, M):
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


class Matrix:
    """
    Reflected and Transmitted matrices
    n - tally number
    """
    def __init__(self, particle):
        self.particle = particle
        tallyDict = {"n" : 1, "e" : 11, "p" : 21 }
        self.tally = tallyDict[particle]
        self.vecT = []
        self.vecR = []
        self.raws = []

    def append(self, mctal, E0, mu0):
        """
        Add mctal.root to the list of raws
        """
        self.raws.append(Data(mctal, E0, mu0, self.tally))

    def run(self):
        """Generate the Reflection and Transmission matrices
        """
        # sort raws by incident energy and direction
        self.raws.sort(key=lambda x : x.E0+x.mu0, reverse=False)

        for raw in self.raws:
            print(raw.E0, raw.mu0)
            for val in raw.T:
                self.vecT.append(val)
            for val in raw.R:
                self.vecR.append(val)

        self.N = len(raw.T)


        self.T = ROOT.TMatrixD(self.N, self.N, array('d',self.vecT))
        self.R = ROOT.TMatrixD(self.N, self.N, array('d',self.vecR))
