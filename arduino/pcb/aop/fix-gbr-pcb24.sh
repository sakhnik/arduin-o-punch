#!/bin/bash

rename F_ Top *
rename B_ Bot *
mv aop-Edge_Cuts.gbr aop-Board.gbr
mv aop-NPTH.drl aop-NoPlated.drl
mv aop-PTH.drl aop-Plated.drl
