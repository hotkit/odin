
SHORTOPTS = 'd:h:'
OPTMAP = {
        '-d': 'dbname',
        '-h': 'host',
    }


def makedsn(opts, args):
    dsnargs = {}
    for arg, opt in OPTMAP.items():
        if arg in opts:
            dsnargs[opt] = opts[arg]
    return ' '.join(['%s=%s' % (n, v) for n, v in dsnargs.items()])

