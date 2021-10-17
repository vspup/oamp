# set the Workspace
setws ./sw_vitis  

# create rtapp
app create -name rtapp -template {OpenAMP echo-test} -hw ./rd/TE_RD/test_board_3eg_1e_2gb.xsa -proc psu_cortexr5_0
platform active test_board_3eg_1e_2gb

# create the petalinux domain
domain create -name "linux_a53" -os linux -proc psu_cortexa53

# generate the platform
platform generate

# create plapp
app create -name plapp -template {Linux Empty Application}  -proc psu_cortexr5 -domain linux_a53

# import header
importprojects ./rd/mps2/
