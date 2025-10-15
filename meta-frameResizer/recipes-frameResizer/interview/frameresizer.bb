SUMMARY = "A mp4 frame publisher"
DESCRIPTION = "This recipe builds a C++ that pulls frames from an mp4 file and publishes them via zeroMQ"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=030cb33d2af49ccebca74d0588b84a21"
SECTION = "resideo"

SRC_URI = "file://frameResizer.cpp \
           file://CMakeLists.txt \
           file://frameResizer.service \
           file://LICENSE"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += " opencv zeromq"
DEPENDS += " frame"
RDEPENDS_${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs"

FILES_${PN} += "${systemd_system_unitdir}/frameResizer.service"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "frameResizer.service"




inherit cmake
