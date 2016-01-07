
all:
	if [ ! -f ../../out/binutils/dumpwrap.o ] ; then	\
	    make ../../out/binutils/dumpwrap.o	;	fi

../../out/binutils/dumpwrap.o:dumpwrap.c dumpwrap.h ../../out/binutils/libbfd.a
	make -C ../../tmp/binutils_build/binutils/ dumpwrap.o
	cp ../../tmp/binutils_build/binutils/dumpwrap.o ../../out/binutils/

../../out/binutils/libbfd.a:../../tmp/binutils_build/Makefile ../../out/binutils/
	make -C ../../tmp/binutils_build/ -j8
	cp -a ../../tmp/binutils_build/bfd/libbfd.a  ../../out/binutils/
	cp -a ../../tmp/binutils_build/libiberty/libiberty.a  ../../out/binutils/
	cp -a ../../tmp/binutils_build/opcodes/libopcodes.a  ../../out/binutils/

../../tmp/binutils_build/Makefile:binutils-2.20.1/ ../../tmp/binutils_build/
	cd ../../tmp/binutils_build/ \
	&& ../../code/binutils/binutils-2.20.1/configure \
	--enable-shared \
	--with-mmap  --disable-nls --enable-64-bit-bfd \
	--enable-targets=i686-pc-linux-gnu,x86_64-unknown-linux-gnu,\
	powerpc-linux-gnu,\
	mips-linux-gnu,mips64-linux-gnu,\
	arm-linux,armeb-linux 
	@echo "finish configure binutils "


../../out/binutils/:
	mkdir -p ../../out/binutils/

binutils-2.20.1/:
	tar xjf binutils-2.20.1.tar.bz2
	ln -s ./binutils-2.20.1/ ./binutils
	patch -p0  < make.patch
	cd binutils-2.20.1/binutils/ && ln -s ./../../dumpwrap.* .

../../tmp/binutils_build/:
	mkdir -p ../../tmp/binutils_build/

#	根据binutils编译输出,手工指定dumpwrap的编译参数
dbg:
	@cd ../../tmp/binutils_build/binutils/ && rm -f ../../../out/binutils/dumpwrap.o && gcc -DHAVE_CONFIG_H -I. -I../../../code/binutils/binutils-2.20.1/binutils  -I. -I../../../code/binutils/binutils-2.20.1/binutils -I../bfd -I../../../code/binutils/binutils-2.20.1/binutils/../bfd -I../../../code/binutils/binutils-2.20.1/binutils/../include -D__USE_MINGW_FSEEK -DLOCALEDIR="\"/usr/local/share/locale\"" -Dbin_dummy_emulation=bin_vanilla_emulation  -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wno-format -Werror -g -D__USE_MINGW_ACCESS -MT dumpwrap.o -MD -MP -MF .deps/dumpwrap.Tpo -c -o dumpwrap.o  ../../../code/binutils/binutils-2.20.1/binutils/dumpwrap.c && cp dumpwrap.o ../../../out/binutils/


