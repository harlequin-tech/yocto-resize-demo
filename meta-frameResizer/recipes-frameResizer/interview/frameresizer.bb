SUMMARY = "A mp4 frame publisher"
DESCRIPTION = "This recipe builds a C++ that pulls frames from an mp4 file and publishes them via zeroMQ"
LICENSE = "CLOSED"
SECTION = "resideo"

SRC_URI = "file://frameResizer.cpp \
           file://CMakeLists.txt"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += " opencv zeromq"
DEPENDS += " frame"

RDEPENDS_${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs"

inherit cmake
