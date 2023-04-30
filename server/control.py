
import keyboard
from typing import Any

def main(*args: Any) -> None:
    while 1:
        if keyboard.is_pressed('w'):
            print(':))')

if __name__ == '__main__':
    import sys

    main(*sys.argv[1:])
