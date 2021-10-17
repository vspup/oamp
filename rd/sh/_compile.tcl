# set the Workspace
setws ./sw_vitis 

# set active platform
platform active test_board_3eg_1e_2gb

# configure the applications
app config -name rtapp build-config release
app config -name plapp build-config release

# build the applications
app build -name rtapp
app build -name plapp