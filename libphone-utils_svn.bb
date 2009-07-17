DESCRIPTION = "phone-utils library"
SECTION = "libs"
PV = "0.0.2+gitr${SRCPV}"
PR = "r1"

DEPENDS="glib-2.0"

inherit pkgconfig autotools autotools_stage

#CONFFILES_${PN} = "${sysconfdir}/phoneg-utils.conf"
SRC_URI = "svn:///home/tom/projects/svn/libphone-utils/;module=trunk;proto=file;rev=HEAD"
