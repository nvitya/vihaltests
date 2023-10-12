#!/bin/bash
./mkvrofs cyw43439_fw.vrofs 43439A0.bin 43439A0_clm.bin
python3 uf2conv.py -b 0x1C0000 -o cyw43439_fw.uf2 cyw43439_fw.vrofs