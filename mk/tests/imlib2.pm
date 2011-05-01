sub Test
{
	my ($ver, $pfx) = @_;
	
	MkExecPkgConfig($pfx, 'imlib2', '--modversion', 'IMLIB2_VERSION');
	MkExecPkgConfig($pfx, 'imlib2', '--cflags', 'IMLIB2_CFLAGS');
	MkExecPkgConfig($pfx, 'imlib2', '--libs', 'IMLIB2_LIBS');
	MkIfNE('${IMLIB2_VERSION}', '');
		MkCompileC('HAVE_IMLIB2', '${IMLIB2_CFLAGS}', '${IMLIB2_LIBS}', << 'EOF');
#include <stdlib.h>
#include <stdio.h>
#include <Imlib2.h>

int
main(int argc, char *argv[])
{
	imlib_context_set_display(NULL);

	return (0);
}
EOF
		MkSaveIfTrue('${HAVE_IMLIB2}', 'IMLIB2_CFLAGS', 'IMLIB2_LIBS');
	MkElse;
		MkPrint('no');
		MkSaveUndef('HAVE_IMLIB2');
	MkEndif;
	return (0);
}

BEGIN
{
	$DESCR{'imlib2'} = 'Imlib2 library (http://docs.enlightenment.org/api/imlib2/html)';
	$TESTS{'imlib2'} = \&Test;
	$DEPS{'imlib2'} = 'cc,x11';
}

;1
