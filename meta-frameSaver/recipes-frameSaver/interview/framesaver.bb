SUMMARY = "A mp4 saver"
DESCRIPTION = "This recipe builds a C++ that receives frames from a resizer and writes them to an mp4"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=030cb33d2af49ccebca74d0588b84a21"
SECTION = "resideo"

SRC_URI = "file://frameSaver.cpp \
           file://CMakeLists.txt \
           file://LICENSE"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += " opencv zeromq"
DEPENDS += " frame"

RDEPENDS_${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs"

inherit cmake
