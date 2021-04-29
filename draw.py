#!/usr/bin/env python

import sys
import ROOT
from data import Data

ROOT.PyConfig.IgnoreCommandLineOptions = True

def main():
    """ """
    ROOT.gStyle.SetOptStat(0)

    c1 = ROOT.TCanvas()
    c1.Divide(3,2)

    fname = "case002/mctal.root"
    n = Data(fname, 1)
    c1.cd(1)
    n.T.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(4)
    n.R.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    e = Data(fname, 11)
    c1.cd(2)
    e.T.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(5)
    e.R.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    p = Data(fname, 21)
    c1.cd(3)
    p.T.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(6)
    p.R.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    p.PrintEbins(p.T)

    input()

if __name__ == "__main__":
    sys.exit(main())
