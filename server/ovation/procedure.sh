#!/bin/bash
#input image size: 800²
#Target image size: 240²

wget https://services.swpc.noaa.gov/images/animations/ovation-north/latest.jpg -O ovation.jpg

#convert latest.jpg -crop 600X600+95+100 +repage cropped.jpg
#mogrify -rotate "90" cropped.jpg 


#Combined:
mogrify -crop 600X600+95+100 +repage -rotate "90" -resize 240x240 ovation.jpg

