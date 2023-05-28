""" Data analysis classes for the GAM method

"""

__title__   = 'GAM'
__version__ = '0.1'
__author__  = 'Konstantin Batkov'

import ROOT
from sys import exit
from os.path import basename
from math import log10
from array import array
from glob import glob
from mctools.common.CombLayer import getPar as getParCL

class Base:
    def buildRow(self, hist):
        """ Build a _sorted_ row from TH2:
        inner loop is energy, outer loop is direction consine.

        """
        nx = hist.GetNbinsX()
        ny = hist.GetNbinsY()
        row = []
        for j in range(1,ny+1):
            for i in range(1,nx+1):
                row.append(hist.GetBinContent(i,j))
        return row

class Data(Base):
    """Convert THnSparse into rows of Reflected and Transmitted
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
        self.fname = fname  # tmp - remove me

        f = ROOT.TFile(fname)
        tally = f.Get("f%d" % n)

        # Reflected
        tally.GetAxis(0).SetRange(1,1); # surf 1
        tally.GetAxis(5).SetRangeUser(-1.0+epsilon, -epsilon) # back
        self.histR = tally.Projection(5,6)
        self.histR.SetNameTitle("R", "Reflected %s;Energy [MeV];#mu" % tally.GetTitle())
        self.histR.SetDirectory(0)
        self.R = self.buildRow(self.histR)

        # Transmitted
        tally.GetAxis(0).SetRange(2,2); # surf 2
        tally.GetAxis(5).SetRangeUser(epsilon, 1.0-epsilon) # forward
        self.histT = tally.Projection(5,6)
        self.histT.SetNameTitle("T", "Transmitted %s;Energy [MeV];#mu" % tally.GetTitle())
        self.histT.SetDirectory(0)
        self.T = self.buildRow(self.histT)

        f.Close()

        if len(self.R) != len(self.T):
            print("Error: R and T have different lengths", len(self.R), len(self.T))
            exit(1)

    def __str__(self):
        return "E0: %g MeV\t μ0: %g\t %s" % (self.E0, self.mu0, self.fname)

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
        self.particle = particle # scored particle
        tallyDict = {"n" : 1, "p" : 11, "e" : 21, "|" : 31 }
        self.tally = tallyDict[particle]
        self.vecT = []
        self.vecR = []
        self.rows = []
        self.energies = set()
        self.directions = set()

    def __str__(self):
        return "Particle: %s" % (self.particle)

    def append(self, mctal, E0, mu0):
        """
        Add mctal.root to the list of rows
        """
        self.rows.append(Data(mctal, E0, mu0, self.tally))

    def run(self):
        """Generate the Reflection and Transmission matrices """

        assert len(self.rows) != 0, "Matrix is empty. Fill it row by row with the Matrix::append method. Check that the transported particle list (-par argument) matches the MCNP mode."

        # sort rows by incident energy and direction
        # 1e6 needed to avoid problems when energies are very close to each other
        self.rows.sort(key=lambda x : x.E0+1e6*abs(x.mu0), reverse=False)

        for row in self.rows:
            self.energies.add(row.E0)
            self.directions.add(row.mu0)
            for val in row.T:
                self.vecT.append(val)
            for val in row.R:
                self.vecR.append(val)

        self.N = len(row.T)

        self.T = ROOT.TMatrixD(self.N, self.N, array('d',self.vecT))
        self.R = ROOT.TMatrixD(self.N, self.N, array('d',self.vecR))

class Source(Base):
    """ sdef definition """

    def __init__(self, hist):
        self.hist = hist.Clone()
        s = self.buildRow(hist)

        self.S = ROOT.TVectorD(len(s), array('d', s))

    def __copy__(self):
        return type(self)(self.hist)

    def fillHist(self):
        """ Fill TH2 based on TVectorD """
        h = self.hist #.Clone(self.hist+"_filled")
        h.Reset()
        nx = h.GetNbinsX()
        ny = h.GetNbinsY()
        for j in range(ny):
            for i in range(nx):
                h.SetBinContent(i+1,j+1, self.S[i+j*nx])
        return h


def readData(files, particles, inpname='inp'):
    """ Build matrices from mctal files.

        files : list of case*/mctal.root files
        particles : list of incident and scored particles
        inpname : input file name in the case* folders   (inp)

        Return dictionary with len(particles)*len(particles) matrices

    """

    m = {}

    for p0 in particles: # incident
        m[p0] = {}
        for p in particles: # scored
            m[p0][p] = Matrix(p)

    for mctal in glob(files):
        print(mctal)
        inp = mctal.replace(basename(mctal), inpname)
        p0  = getParCL(inp, "Incident particle:", 3)
        E0  = getParCL(inp, "Energy:")
        mu0 = getParCL(inp, "Direction cosine:", 3)
        for p in particles:
            m[p0][p].append(mctal,E0,mu0)

    for p0 in particles: # incident
        for p in particles: # scored
            m[p0][p].run()

    return m
