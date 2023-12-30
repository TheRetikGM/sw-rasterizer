BUILDDIR=builddir

build:
	meson compile -C ${BUILDDIR}

setup:
	meson setup ${BUILDDIR}

run:
	cd ${BUILDDIR} && ninja -j32 && src/main

clean:
	cd ${BUILDDIR} && ninja clean

purge:
	rm -rf ${BUILDDIR}

.PHONY: setup build run clean purge
