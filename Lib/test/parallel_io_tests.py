import os

def test_import():
    try:
        import os
        import math
        from math import cos
        from glob import glob
        from re import match, search
    except ImportError:
        return 'FAIL'
    return 'OK'

def test_fileopen(filename):
    try:
        fp = open(filename)
        fp.close()
    except IOError:
        return 'FAIL'
    return 'OK'

def test_fileread(filename, reference=None):
    try:
        with open(filename) as fp:
            content = fp.read()
            if reference is not None:
                assert content==reference
    except AssertionError:
        return 'FAIL'
    return 'OK'

def test_filestat(filename, exists):
    try:
        exists = bool(exists)
        assert exists == os.path.isfile(filename)
    except AssertionError:
        return 'FAIL'
    return 'OK'

# reference text from 'lorem-ipsum.txt'
lorem_file = os.path.join(os.path.dirname(__file__), 'lorem-ipsum.txt')
lorem = \
"""Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Sed posuere
interdum sem. Quisque ligula eros ullamcorper quis, lacinia quis facilisis
sed sapien. Mauris varius diam vitae arcu. Sed arcu lectus auctor vitae,
consectetuer et venenatis eget velit. Sed augue orci, lacinia eu tincidunt
et eleifend nec lacus. Donec ultricies nisl ut felis, suspendisse potenti.
Lorem ipsum ligula ut hendrerit mollis, ipsum erat vehicula risus, eu
suscipit sem libero nec erat. Aliquam erat volutpat. Sed congue augue vitae
neque. Nulla consectetuer porttitor pede. Fusce purus morbi tortor magna
condimentum vel, placerat id blandit sit amet tortor.
"""
bogus_file = os.path.join(os.path.dirname(__file__), 'no-such-file')

# run tests
print('Testing parallel I/O wrappers.')
print('  I/O wrappers are ON (default)')
print('    import:    {0}'.format(test_import()))
print('    file stat: {0}'.format(test_filestat(lorem_file, True)))
print('      no file: {0}'.format(test_filestat(bogus_file, False)))
print('    file open: {0}'.format(test_fileopen(lorem_file)))
print('    file read: {0}'.format(test_fileread(lorem_file, lorem)))
wrapoff()
print('  I/O wrappers are OFF')
print('    import:    {0}'.format(test_import()))
print('    file stat: {0}'.format(test_filestat(lorem_file, True)))
print('      no file: {0}'.format(test_filestat(bogus_file, False)))
print('    file open: {0}'.format(test_fileopen(lorem_file)))
print('    file read: {0}'.format(test_fileread(lorem_file, lorem)))
wrapon()
print('  I/O wrappers are ON again')
print('    import:    {0}'.format(test_import()))
print('    file stat: {0}'.format(test_filestat(lorem_file, True)))
print('      no file: {0}'.format(test_filestat(bogus_file, False)))
print('    file open: {0}'.format(test_fileopen(lorem_file)))
print('    file read: {0}'.format(test_fileread(lorem_file, lorem)))

