#/bin/bash
ovation/procedure.sh
visible/procedure.sh
mag/procedure.sh
xray/procedure.sh
convert visible.jpg mag.jpg xray.jpg +append -rotate "90" sidebar.jpg
convert ovation.jpg sidebar.jpg +append full.bmp

echo "Done."
exit 0