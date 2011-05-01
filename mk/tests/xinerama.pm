
sub Test
{
	my ($ver, $pfx) = @_;
	
	MkExecPkgConfig($pfx, 'xinerama', '--modversion', 'XINERAMA_VERSION');
	MkExecPkgConfig($pfx, 'xinerama', '--cflags', 'XINERAMA_CFLAGS');
	MkExecPkgConfig($pfx, 'xinerama', '--libs', 'XINERAMA_LIBS');
	MkIfNE('${XINERAMA_VERSION}', '');
		MkCompileC('HAVE_XINERAMA', '${XINERAMA_CFLAGS}', '${XINERAMA_LIBS}', << 'EOF');
#include <stdlib.h>
#include <stdio.h>
#include <X11/extensions/Xinerama.h>

int
main(int argc, char *argv[])
{
	XineramaQueryExtension(NULL, NULL, NULL);

	return (0);
}
EOF
		MkSaveIfTrue('${HAVE_XINERAMA}', 'XINERAMA_CFLAGS', 'XINERAMA_LIBS');
	MkElse;
		MkPrint('no');
		MkSaveUndef('HAVE_XINERAMA');
	MkEndif;
	return (0);
}

BEGIN
{
	$DESCR{'xinerama'} = 'X11 Xinerama library';
	$TESTS{'xinerama'} = \&Test;
	$DEPS{'xinerama'} = 'cc,x11';
}

;1
