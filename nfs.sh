#mount the nfs filesystem to the /mnt/nfs ,the src ip is 192.168.0.46
echo mount the 192.168.0.46:/home/nfs to the /mnt/nfs
mount -t nfs -o nolock 192.168.0.46:/home/nfs /mnt/nfs