#!/bin/bash
set -e

IMG_FILE="./build/harddisk.img"
IMG_SIZE_MB=32
MNT_POINT="/mnt/learn-os-temp"

MBR_BIN="./build/boot.bin"
VBR_BIN="./build/stage2.bin"
KERNEL_BIN_SRC="./build/kernel_low.bin"
KERNEL_BIN_DST="KERNELL.BIN"

cleanup() {
    echo "--- Cleaning up ---"
    if mountpoint -q "$MNT_POINT"; then
        echo "Unmounting $MNT_POINT..."
        sudo umount "$MNT_POINT"
    fi

    if [ -d "$MNT_POINT" ]; then
        echo "Removing temporary mount point..."
        sudo rmdir "$MNT_POINT"
    fi

    if [ -n "$LOOP_DEV" ] && [ -b "$LOOP_DEV" ]; then
        echo "Detaching loop device $LOOP_DEV..."
        sudo losetup -d "$LOOP_DEV"
    fi
    echo "Cleanup complete."
}

trap cleanup EXIT

echo "--- Starting OS Image Build ---"

echo "Creating ${IMG_SIZE_MB}MB hard disk image: $IMG_FILE"
dd if=/dev/zero of="$IMG_FILE" bs=1M count=$IMG_SIZE_MB

echo "Writing MBR to $IMG_FILE"
dd if="$MBR_BIN" of="$IMG_FILE" bs=512 count=1 conv=notrunc

echo "Setting up loopback device..."
LOOP_DEV=$(sudo losetup -fP --show "$IMG_FILE")
PARTITION="${LOOP_DEV}p1"
echo "Image mounted to $LOOP_DEV, partition is $PARTITION"

echo "Formatting $PARTITION with FAT16 (reserving 256 sectors)..."
sudo mkfs.fat -F 16 -R 128 "$PARTITION"

echo "Writing VBR jump and code to $PARTITION (preserving BPB)..."
sudo dd if="$VBR_BIN" of="$PARTITION" bs=1 count=3 conv=notrunc
sudo dd if="$VBR_BIN" of="$PARTITION" bs=1 skip=62 seek=62 conv=notrunc

echo "Writing kernel ($KERNEL_BIN_SRC) to $PARTITION..."
sudo dd if="$KERNEL_BIN_SRC" of="$PARTITION" bs=512 seek=1 conv=notrunc

echo "Creating temporary mount point at $MNT_POINT"
sudo mkdir -p "$MNT_POINT"
sudo mount "$PARTITION" "$MNT_POINT"
echo "Partition mounted to $MNT_POINT"

echo "Copying additional files to the filesystem..."
echo "Hello from FAT16!" | sudo tee "$MNT_POINT/TEST.TXT" > /dev/null
sudo mkdir "$MNT_POINT/DESKTOP"

if [ -f "./fortune.txt" ]; then
    sudo cp ./fortune.txt "$MNT_POINT/FORTUNET.TXT"
fi

if [ -f "./GOSPELS.TXT" ]; then
    sudo cp ./GOSPELS.TXT "$MNT_POINT/M.IBS"
fi

sudo cp ./build/hello.bin "$MNT_POINT/HELLOEXE.IKE"
sudo cp ./build/editor.bin "$MNT_POINT/EDITOR.IKE"
sudo cp ./build/basic.bin "$MNT_POINT/BASIC.IKE"
sudo cp ./build/gui2.bin "$MNT_POINT/GUI.bin"

if [ -d "./assets/txt" ]; then
    for file in ./assets/txt/*.txt; do
        if [ -f "$file" ]; then
            sudo cp "$file" "$MNT_POINT/DESKTOP/"
        fi
    done
fi

echo "--- Build successful: $IMG_FILE ---"