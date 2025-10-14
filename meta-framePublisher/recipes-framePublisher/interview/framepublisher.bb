SUMMARY = "A mp4 frame publisher"
DESCRIPTION = "This recipe builds a C++ that pulls frames from an mp4 file and publishes them via zeroMQ"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=030cb33d2af49ccebca74d0588b84a21"
SECTION = "resideo"

SRC_URI = "file://framePublisher.cpp \
           file://CMakeLists.txt \
           file://sintel-1024-surround.mp4 \
           file://LICENSE"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += " opencv zeromq"
DEPENDS += " frame"

RDEPENDS_${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs"

FILES:${PN} = "/usr /usr/share /usr/bin/framePublisher"

#do_install() {
#    install -d ${D}/usr/share
#    #install -m 0555 sintel-1024-surround.mp4 ${D}/usr/share
#}

inherit cmake
