#!/bin/bash
#input: 512²
#output: 80²
wget https://sdo.gsfc.nasa.gov/assets/img/latest/latest_512_HMIB.jpg -O mag.jpg
mogrify -crop -20-20 +repage -resize 72x72 -background black -gravity center -extent 80x80 mag.jpg

#-background black -gravity center -scale 80x80 -extent 80x80