#
# This file is the rtmsg recipe.
#

SUMMARY = "Simple rtmsg application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://rtapp"


S = "${WORKDIR}"
INSANE_SKIP_${PN} = "arch"

do_install() {
	install -d ${D}/lib/firmware             
	install -m 0644 ${S}/rtapp ${D}/lib/firmware/rtapp
}

FILES_${PN} = "/lib/firmware/rtapp"
