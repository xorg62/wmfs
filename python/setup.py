from distutils.core import setup, Extension

module1 = Extension('wmfs',
                    include_dirs = ['/usr/local/include'],
                    libraries = ['X11'],
                    library_dirs = ['/usr/local/lib'],
                    sources = ['screen_func.c',
                               'tag_func.c',
                               'libwmfs.c'])

setup (name = 'wmfs',
       version = '0.1',
       description = 'WMFS client library',
       author = 'Martin Duquesnoy',
       author_email = '',
       url = 'http://wmfs.sangor.net/',
       long_description = '''
This is a control librarie for the WM named WMFS.
''',
       ext_modules = [module1])

