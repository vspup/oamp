#! /bin/sh

# name petalinux assembly
pl_folder=plinux

# check xilinx installation
PLDIR=/home/volod/xilinx/Petalinux/2020.2
VITISDIR=/home/volod/xilinx/Vitis/2020.2

source ${PLDIR}/settings.sh 
source ${VITISDIR}/settings64.sh

xsct ./_compile.tcl  

echo ""
echo "--  Update app files"
echo "--  plapp"
cp ./sw_vitis/plapp/Release/plapp.elf ./${pl_folder}/project-spec/meta-user/recipes-apps/plapp/files/
cp ./${pl_folder}/project-spec/meta-user/recipes-apps/plapp/files/plapp.elf ./${pl_folder}/project-spec/meta-user/recipes-apps/plapp/files/plapp
rm ./${pl_folder}/project-spec/meta-user/recipes-apps/plapp/files/plapp.elf
echo "--  rtapp"
cp ./sw_vitis/rtapp/Release/rtapp.elf ./${pl_folder}/project-spec/meta-user/recipes-apps/rtapp/files/
cp ./${pl_folder}/project-spec/meta-user/recipes-apps/rtapp/files/rtapp.elf ./${pl_folder}/project-spec/meta-user/recipes-apps/rtapp/files/rtapp
rm ./${pl_folder}/project-spec/meta-user/recipes-apps/rtapp/files/rtapp.elf



echo ""
echo ""
echo "--  Re Build pl" 
cd ./${pl_folder} 
petalinux-build


echo ""
echo ""
echo "-- Package pl" 

echo ""
echo "--  cd to image/linux directory"
cd ./images/linux

echo ""
echo "--  Package linux"
petalinux-package --boot --fsbl fsbl.elf --fpga system.bit  --u-boot u-boot.elf --force

	