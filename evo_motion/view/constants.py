from os.path import abspath, dirname, join
from typing import Final

SHADER_ROOT_PATH: Final[str] = abspath(join(dirname(__file__), "shaders"))

BYTES_PER_FLOAT: Final[int] = 4
