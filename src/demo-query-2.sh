#!/bin/bash

./euclid-cli \
" with " \
" member measure.[################ successful ################] as 660880 " \
" member measure.[############################################] as 660880 " \
" select " \
" { (Calendar.[ALL]) } on 0, " \
" { (measure.[############################################]), " \
" (measure.[################ successful ################]), " \
" (measure.[############################################]) } on 1 " \
" from [Online Store] "