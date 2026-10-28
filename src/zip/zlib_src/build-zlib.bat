
@set ZLIBDIR=zlib-1.3.1

cd %ZLIBDIR%


@rem --- make x86_64

make -f win32/Makefile.gcc clean
make -f win32/Makefile.gcc

@set DESTDIR=..\..\win

mkdir %DESTDIR%
copy libz.a     %DESTDIR%\libz-64.a
copy libz.dll.a %DESTDIR%\libz-64.dll.a
copy zlib1.dll  %DESTDIR%\zlib1-64.dll


@rem --- make x86

make -f win32/Makefile.gcc clean
make -f win32/Makefile.gcc CFLAGS=-m32
copy libz.a     %DESTDIR%\libz-32.a
copy libz.dll.a %DESTDIR%\libz-32.dll.a
copy zlib1.dll  %DESTDIR%\zlib1-32.dll


make -f win32/Makefile.gcc clean

# ------------------------
# grab standard zip wrappers code

copy ioapi.c ..\..\
copy ioapi.h ..\..\
copy iowin32.c ..\..\
copy iowin32.h ..\..\
copy unzip.c ..\..\
copy unzip.h ..\..\
copy zconf.h ..\..\
copy zlib.h ..\..\


exit /b
