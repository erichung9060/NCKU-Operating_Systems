make

sudo insmod osfs.ko
mkdir mnt
sudo mount -t osfs none mnt/
cd mnt

sudo touch test1.txt
ls
sudo bash -c "echo 'I love OSLAB' > test1.txt"
cat test1.txt

cd ..
sudo umount mnt/
sudo rmmod osfs
sudo rm -r mnt

make clean