export MODULE_NAME=vtagfs
export MOUNT_PATH=/mnt/vtagfs

lsmod | grep $MODULE_NAME

mkdir -p $MOUNT_PATH

insmod $MODULE_NAME.ko
mount -t $MODULE_NAME none $MOUNT_PATH

./user/user_query

dmesg | tail -20 | less

umount $MOUNT_PATH
rmmod $MODULE_NAME

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
zip -r tagfs_err39.zip tagfs
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
