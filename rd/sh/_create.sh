#! /bin/sh

# ****
PLDIR=/home/volo/xilinx/Petalinux/2020.2
VITISDIR=/home/volo/xilinx/Vitis/2020.2
DLSTR="DL_DIR = \"/home/volo/xilinx/Petalinux/2020.2/yocto/downloads\""
SSSTR="SSTATE_DIR = \"/home/volo/xilinx/Petalinux/2020.2/yocto/sstate-cache\""
# ****



# name petalinux assembly
pl_folder=plinux
echo "-- 0. name of project '${pl_folder}' "
# petalinux setting
source ${PLDIR}/settings.sh 


echo ""
echo ""
echo "--  1. create peta linux (pl) project " 

petalinux-create --type project --template zynqMP --name ${pl_folder} --force
#mkdir ./${pl_folder}

cp ./rd/TE_RD/test_board_3eg_1e_2gb.xsa ./${pl_folder}
cp -r ./rd/plinux_s/* ./${pl_folder}

cd ./${pl_folder}

petalinux-config --get-hw-description --silentconfig


echo ""
echo "--  petalinuxbsp.conf "
cd ../
echo ${DLSTR} >> ./${pl_folder}/project-spec/meta-user/conf/petalinuxbsp.conf
echo ${SSSTR} >>./${pl_folder}/project-spec/meta-user/conf/petalinuxbsp.conf



echo ""
echo ""
echo "--  2. Vitis SW" 
source ${VITISDIR}/settings64.sh

mkdir ./sw_vitis
cp ./rd/TE_RD/test_board_3eg_1e_2gb.xsa ./sw_vitis/

xsct ./_sw.tcl

echo ""
echo "--  copu src plapp"
cp ./rd/plapp_s/*.* ./sw_vitis/plapp/src/

echo ""
echo "--  copy src rtapp"
cp ./rd/rtapp_s/*.* ./sw_vitis/rtapp/src/
rm ./sw_vitis/rtapp/src/rpmsg-echo.h
rm ./sw_vitis/rtapp/src/rpmsg-echo.c

xsct ./_compile.tcl

echo ""
echo ""
echo "--  3. Update app files"
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
echo "--  4. Build pl" 
cd ./${pl_folder} 
petalinux-build

echo ""
echo ""
echo "--  5. Package pl" 
echo ""
echo "--  fsbl "
cd ../
cp ./rd/TE_RD/fsbl.elf ./${pl_folder}/images/linux

echo ""
echo "--  scr "
cp ./rd/TE_RD/boot.scr ./${pl_folder}/images/linux

echo ""
echo "--  cd to image/linux directory"
cd ./${pl_folder}/images/linux

echo ""
echo "--  Package linux"
petalinux-package --boot --fsbl fsbl.elf --fpga system.bit  --u-boot u-boot.elf --force

	