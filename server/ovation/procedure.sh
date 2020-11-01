#!/bin/bash
#input image size: 800²
#Target image size: 240²

#wget https://services.swpc.noaa.gov/images/animations/ovation-north/latest.jpg -O ovation.jpg
wget https://services.swpc.noaa.gov/images/animations/ovation/north/latest.jpg -O ovation.jpg


#get current KP index (tail gets latest line, sed outputs everything after the last space in the line, head outputs current KP)
#KP=$(curl -s https://services.swpc.noaa.gov/text/daily-geomagnetic-indices.txt | tail -n 1 | sed 's/.*\ //' | head -c 1 2>&1)
KP=$(curl https://services.swpc.noaa.gov/json/planetary_k_index_1m.json | jq ".[-1] | .kp_index")
#colorize kp index text:
# http://www.aurora-service.eu/aurora-school/all-about-the-kp-index/
kpColour=green
if [ $((KP)) -ge 4 ]
then
kpColour=yellow
fi
if [ $((KP)) -ge 6 ]
then
kpColour=red
fi


#Combined:                                                                                                       Horizontal line               Vertical line              Marker circle                            Circle center Marker                                     #add kp index text
#mogrify -crop 600X600+95+100 +repage -rotate "90" -resize 240x240 -fill white -stroke white -strokewidth 1 -draw "line 151,0 151,240" -draw "line 0,206 239,206" -draw "fill none circle 97,135 151,206" -draw "fill red circle 96.5,134.5 97.5,135.5" -stroke $kpColour -fill $kpColour -strokewidth 1 -pointsize 25 -annotate +10+30 "kp: $KP" ovation.jpg
mogrify -crop 580X580+95+75 +repage -rotate "90" -resize 240x240 -fill white -stroke white -strokewidth 1 -draw "line 160,0 160,240" -draw "line 0,213 239,213" -draw "fill none circle 111,139 160,213" -draw "fill red circle 110.5,138.5 111.5,139.5" -stroke $kpColour -fill $kpColour -strokewidth 1 -pointsize 25 -annotate +10+30 "kp: $KP" ovation.jpg
