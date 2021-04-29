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

    fname = "water/case002/mctal.root"
    n = Data(fname, 1)
    c1.cd(1)
    n.histT.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(4)
    n.histR.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    e = Data(fname, 11)
    c1.cd(2)
    e.histT.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(5)
    e.histR.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    p = Data(fname, 21)
    c1.cd(3)
    p.histT.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()
    c1.cd(6)
    p.histR.Draw("col")
    ROOT.gPad.Modified()
    ROOT.gPad.Update()
    ROOT.gPad.SetLogx()

    print(p.R)
    print(len(p.R))

#    p.PrintEbins(p.histT)

    input()

if __name__ == "__main__":
    sys.exit(main())
