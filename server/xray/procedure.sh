#!/bin/bash
#input: 512²
#output: 80²
wget https://sdo.gsfc.nasa.gov/assets/img/latest/latest_512_0094.jpg -O xray.jpg
mogrify -crop -20-20 +repage -resize 80x80 -rotate "-90" xray.jpg

