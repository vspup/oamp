# oamp
Project for testing the development of the openAMP system on te0821+te0701.

Requirements!!!
Vitis 2020.2
Petalinux 2020.2

**Attention!!!

Before cloning the repository, you need to download the archive - https://github.com/vspup/oamp/blob/master/oamp.tar.xz. 
Copy the archive to the folder where the project will be located. 
Unpack the archive in this park. 
You should end up with a file structure like this:
./
  rd
  _compile.tcl
  _create.sh
  oamp.tar.xz
  _sw.tcl
  _update.sh
  .gitignore
  
In the file - _create.sh, you need to make changes in accordance with your location of the Xilinx and Petalinux tools.
  
  PLDIR=/home/volo/xilinx/Petalinux/2020.2
  
  VITISDIR=/home/volo/xilinx/Vitis/2020.2

In addition, these two variables contain the path to the folder in which the downloaded files will be cached.
  
  DLSTR="DL_DIR = \"/home/volo/xilinx/Petalinux/2020.2/yocto/downloads\""
  
  SSSTR="SSTATE_DIR = \"/home/volo/xilinx/Petalinux/2020.2/yocto/sstate-cache\""  

Make the script an executable file and run it to run. This script will create:
- plinux - petalinux project
- sw_vitis - directory with app progects
- rtapp
- plapp

After the script finishes successfully, you can clone the repository to this folder.
