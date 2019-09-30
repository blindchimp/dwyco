rm NDK_ABI*
./config_make_everything.sh
touch NDK_ABI_X86
./config_make_everything.sh
rm NDK_ABI*
touch NDK_ABI_ARM64
./config_make_everything.sh
rm NDK_ABI*
