SUMMARY = "A mp4 frame publisher and subscriber"
DESCRIPTION = "This recipe builds a C++ libraryh that supports reading frames from an mp4 file and publishes them via zeroMQ"
LICENSE = "CLOSED"
SECTION = "resideo"

SRC_URI = "file://frame.cpp \
           file://include/frame.h \
           file://CMakeLists.txt"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += " opencv zeromq"
RDEPENDS_${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs"

inherit cmake

#do_install() {
#    install -d ${D}${libdir}
#    install -m 0755 ${S}/libframe.so ${D}${libdir}
#    install -d ${D}${includedir}
#    install -m 0644 ${S}/frame.h ${D}${includedir}
#}
