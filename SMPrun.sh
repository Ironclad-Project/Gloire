set -e

qemu-system-riscv64 -M virt -cpu rv64 -smp 4 -device ramfb -device qemu-xhci        \
   -device usb-kbd -device usb-mouse -drive if=pflash,unit=0,format=raw,file=../riscv64.fd -hda gloire.iso -serial stdio -m 4G

