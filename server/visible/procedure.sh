#!/bin/bash
#input size: 512²
#output size: 80²
wget https://sdo.gsfc.nasa.gov/assets/img/latest/latest_512_HMII.jpg -O visible.jpg
mogrify -resize 80x80 visible.jpg
