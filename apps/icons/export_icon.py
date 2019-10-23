from subprocess import Popen, PIPE


acorn_png_export_applescript = '''
tell application "Acorn"
    tell document 1
        resize image width %(size)d
        save as PNG in POSIX file "%(path)s"
        undo
    end tell
end tell
'''


def export_icon(size, path):
    cmd = Popen(['/usr/bin/osascript'], stdin=PIPE)
    cmd.communicate((acorn_png_export_applescript %
                     {'size': size, 'path': path}).encode('utf-8'))
    assert cmd.returncode == 0
