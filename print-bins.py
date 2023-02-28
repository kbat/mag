#!/usr/bin/env python

import ROOT

f = ROOT.TFile("mctal.root")
f1 = f.Get("f1")
f1.Print("a")

h = f1.Projection(6)
n = h.GetNbinsX()

x = h.GetXaxis()

print("Centres of bins:")
print("c @@@ ENERGY=", end="")
for i in range(1, n+1):
    print("%.2e" % x.GetBinCenter(i), end=" ")
#    print("%.2e" % x.GetBinLowEdge(i+1), end=" ")
print("")

h = f1.Projection(5)
n = h.GetNbinsX()
if n % 2 != 0:
    print("Number of angular bins must be even: ", n)
    exit(1)

x = h.GetXaxis()

print("c @@@ DIR=", end="")
for i in range(1, n+1):
    val = x.GetBinCenter(i)
    if val>0:
        print("%.2f" % x.GetBinCenter(i), end=" ")
print("")
