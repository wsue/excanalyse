all:../../tmp/ui-build/Makefile
	make -C ../../tmp/ui-build

release:
	make -C ../../tmp/ui-build release

clean:
	rm -rf ../../tmp/ui-build/*

../../tmp/ui-build/Makefile:../../tmp/ui-build
	cd ../../tmp/ui-build && qmake ../../code/ui/excanalyse.pro

../../tmp/ui-build:
	mkdir ../../tmp/ui-build

