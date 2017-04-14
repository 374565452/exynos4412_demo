#config the alsa mixer default vol 
echo --------------start config--------------
echo numid = 5 127 
/usr/local/alsa-utils/bin/amixer cset numid=5 127 
echo numid= 6 1
/usr/local/alsa-utils/bin/amixer cset numid=6 1
echo numid= 30 8,0
/usr/local/alsa-utils/bin/amixer cset numid=30 8,0
echo numid= 40 1 
/usr/local/alsa-utils/bin/amixer cset numid=40 1
echo numid= 43 1
/usr/local/alsa-utils/bin/amixer cset numid=43 1
echo numid= 47 1
/usr/local/alsa-utils/bin/amixer cset numid=47 1
echo -------------end config--------------