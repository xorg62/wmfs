
sub Test
{
	my ($ver, $pfx) = @_;
	
	MkExecPkgConfig($pfx, 'xrandr', '--modversion', 'XRANDR_VERSION');
	MkExecPkgConfig($pfx, 'xrandr', '--cflags', 'XRANDR_CFLAGS');
	MkExecPkgConfig($pfx, 'xrandr', '--libs', 'XRANDR_LIBS');
	MkIfNE('${XRANDR_VERSION}', '');
		MkCompileC('HAVE_XRANDR', '${XRANDR_CFLAGS}', '${XRANDR_LIBS}', << 'EOF');
#include <stdlib.h>
#include <stdio.h>
#include <X11/extensions/Xrandr.h>

int
main(int argc, char *argv[])
{
	XRRQueryExtension(NULL, NULL, NULL);

	return (0);
}
EOF
		MkSaveIfTrue('${HAVE_XRANDR}', 'XRANDR_CFLAGS', 'XRANDR_LIBS');
	MkElse;
		MkPrint('no');
		MkSaveUndef('HAVE_XRANDR');
	MkEndif;
	return (0);
}

BEGIN
{
	$DESCR{'xrandr'} = 'X11 xrandr extension';
	$TESTS{'xrandr'} = \&Test;
	$DEPS{'xrandr'} = 'cc,x11';
}

;1
