qemu-system-riscv64 -M virt -cpu rv64 -smp 4 -nographic -monitor none \
  -serial stdio \
  -drive if=pflash,unit=0,format=raw,file=../riscv64.fd \
  -hda gloire.iso -m 4G

