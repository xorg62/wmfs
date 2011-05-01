
sub Test
{
	my ($ver, $pfx) = @_;
	
	MkExecPkgConfig($pfx, 'xft', '--modversion', 'XFT_VERSION');
	MkExecPkgConfig($pfx, 'xft', '--cflags', 'XFT_CFLAGS');
	MkExecPkgConfig($pfx, 'xft', '--libs', 'XFT_LIBS');
	MkIfNE('${XFT_VERSION}', '');
		MkCompileC('HAVE_XFT', '${XFT_CFLAGS}', '${XFT_LIBS}', << 'EOF');
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xft/Xft.h>

int
main(int argc, char *argv[])
{
	XftFontOpenName(NULL, 0, NULL);
	return (0);
}
EOF
		MkSaveIfTrue('${HAVE_XFT}', 'XFT_CFLAGS', 'XFT_LIBS');
	MkElse;
		MkPrint('no');
		MkSaveUndef('HAVE_XFT');
	MkEndif;
	return (0);
}

BEGIN
{
	$DESCR{'xft'} = 'X11 xft extension';
	$TESTS{'xft'} = \&Test;
	$DEPS{'xft'} = 'cc,x11';
}

;1
