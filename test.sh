export MODULE_NAME=vtagfs
export MOUNT_PATH=/mnt/vtagfs

lsmod | grep $MODULE_NAME

mkdir -p $MOUNT_PATH

insmod $MODULE_NAME.ko
mount -t $MODULE_NAME none $MOUNT_PATH

cat ::red/a1

umount $MOUNT_PATH
rmmod $MODULE_NAME

cat ::red/john/Desktop/asm64.s

ls -lai $MOUNT_PATH/red


dmesg -w

export PS1='> '
# ls -laH red/a2

echo 1 > /proc/sys/vm/drop_caches
echo 2 > /proc/sys/vm/drop_caches
echo 3 > /proc/sys/vm/drop_caches


umount $MOUNT_PATH
rmmod $MODULE_NAME



/root/projects/tagfs


./user/user_query

dmesg | tail -200 | less



ls -lai $MOUNT_PATH/red


DRIVE_NAME="test_ext4"
mkdir $DRIVE_NAME

dd if=/dev/zero of=$DRIVE_NAME.img bs=1M count=50
mkfs.ext4 $DRIVE_NAME.img

sudo mount -o loop -t ext4  $DRIVE_NAME.img $DRIVE_NAME

umount $DRIVE_NAME

/root/projects/tests/test_tags/tagfs/test_ext4/c1/c2/ff1

export MODULE_NAME=vtagfs
export MOUNT_PATH=/mnt/vtagfs

rmmod $MODULE_NAME
insmod $MODULE_NAME.ko

./user/user_query red

dmesg | tail -20 | less


./user/user_query /home/john/tes1/kk1 red
./user/user_query /home/john/tes1/tes1_2/kk1_2 red
./user/user_query red

./user/user_query red
dmesg | tail -20 | less




./user/user_query /home/john/dir1/dir2/dir1/f1 red




rmmod $MODULE_NAME

ls -lai /.tag_v32/red/contain_v32/home/contain_v32

export MODULE_VERSION=12

ls -lai /home/john/dir1/dir2/dir1/.tag_v$MODULE_VERSION/red/this_v$MODULE_VERSION/


ls -lai /home/john/dir1/dir2/.tag_v$MODULE_VERSION/red/contain_v$MODULE_VERSION/
ls -lai /home/john/dir1/dir2/.tag_v$MODULE_VERSION/red/contain_v$MODULE_VERSION/dir1


ls -lai /home/john/dir1/dir2/dir1
ls -lai /home/john/dir1/dir2/dir1/.tag_v2/red/this_v2

ls -lai /home/john/dir1/dir2/.tag_v2/red/contain_v2/
ls -lai /.tag_v1/red/contain_v1/




create_this(file, tag)
    parent = get_perant(file)
    create_dir(parent/".tag"/tag/"this")
    link_file(file, parent/".tag"/tag/"this"/file)

create_contain(dir, child_dir, tag)
    parent = get_perant(dir)
    create_dir(dir/".tag"/tag/"contain")
    link_dir(child_dir, dir/".tag"/tag/"contain"/child_dir)



echo 1 > /proc/sys/vm/drop_caches
echo 2 > /proc/sys/vm/drop_caches
echo 3 > /proc/sys/vm/drop_caches


ls -lai /.tag_u13/red/contain_u13/home/contain_u13/john/contain_u13/tes1/this_u13/kk1

cd ..
zip -r tagfs_err101.zip tagfs
python3 -m http.server
rm -rf tagfs_*.zip
cd tagfs

mkdir -p /home/john/dir1/dir2/dir1
touch /home/john/dir1/dir2/dir1/f1

dmesg -wH













export DBG_VERS=27

touch /home/john/jj$DBG_VERS

./user/user_query /home/john/jj$DBG_VERS gree$DBG_VERS

dmesg | tail -20 | less




############################################# >

mkdir -p test
dd if=/dev/zero of=test_over.img bs=1M count=50
mkfs.ext4 test2.img

sudo mount -o loop -t ext4 test.img test
umount test

dd if=/dev/zero bs=1M count=50 >> test.img

pvcreate /dev/loop15

dd if=/dev/zero of=test2.img bs=1M count=50

sudo losetup --partscan -f test.img
sudo vgscan --scan
sudo vgchange -ay /dev/loop15


mkdir test_va

dd if=/dev/zero of=disk1.img bs=1M count=50
LOOP_DEV=losetup --find --show ./disk1.img
pvcreate $LOOP_DEV

vgcreate vg01 $LOOP_DEV

lvcreate -n lvdata01 -L 20MB vg01
mkfs.ext4 /dev/vg01/lvdata01

mount /dev/vg01/lvdata01 test_va

lvextend /dev/vg01/lvdata01 -L30M

############################################# <
export PS1='> '

make clean && make all


export PS1='> '


export MODULE_NAME=vtagfs
export MOUNT_PATH=/mnt/vtagfs

lsmod | grep $MODULE_NAME

mkdir -p $MOUNT_PATH


insmod $MODULE_NAME.ko
mount -t $MODULE_NAME none $MOUNT_PATH


./user/read_dir ::red52

cat ::red52/bli

ls -lai ::red52
ls ::red52

dmesg -w


umount $MOUNT_PATH
rmmod $MODULE_NAME




./user/user_query /root/projects/tagfs red52
./user/user_query /root/projects/p_1 red52


./user/user_query /home/john/dhh1/hhh2 blue


ls ::blue

ls /mnt/vtagfs/red52/sym1

ls ::red

./user/read_dir ::blue




touch /home/john/dhh1/hhh2


getfattr -d -m ".*" /red46/dmap0003/hhh2

getfattr -d -m ".*" /red28/dmap0002/hhh1
s

setfattr -n "security.ino2" -v "test2" /red46/dmap0002/hhh1



echo 1 > /proc/sys/vm/drop_caches
echo 2 > /proc/sys/vm/drop_caches
echo 3 > /proc/sys/vm/drop_caches

cat /proc/kallsyms | grep -i "sys_openat"