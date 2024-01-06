import os.path
import re
from pathlib import Path
from typing import Optional, Tuple

import tomli


def _package_version_str() -> Optional[str]:
    pyproject_toml_file: Path = Path(__file__).parent / "../pyproject.toml"

    try:
        with open(pyproject_toml_file, "rb") as f:
            return tomli.load(f)["tool"]["poetry"]["version"]
    except IOError as _ioe:
        return None


def _package_version_from_str(version_str: str) -> Tuple[int, int, int]:
    v = re.search(r"^(.*)\.(.*)\.(.*)$", version_str).groups()
    return int(v[0]), int(v[1]), int(v[2])


def _generate_c_files(major: int, minor: int, patch: int):
    c_file = os.path.relpath(Path(__file__).parent / "../../Src/version.c")
    h_file = os.path.relpath(Path(__file__).parent / "../../Inc/version.h")

    autogen = """/** \\file {file}    
 * Auto generated file crated by "poetry run versionbump".
 * See also "poetry version".
 *
 * example:
 * \\code{{.sh}}
 * poetry version patch
 * poetry run versionbump
 * \\endcode
 **/"""

    c_template = f"""{autogen.format(file="version.c")}

#include "version.h"
"""

    h_template = f"""{autogen.format(file="version.h")}

#pragma once

#define VERSION "{major}.{minor}.{patch}"
#define VERSION_MAJOR {major}
#define VERSION_MINOR {minor}
#define VERSION_PATCH {patch}
"""

    with open(h_file, "w") as f:
        print(f"writing version {major}.{minor}.{patch} to {h_file}")
        f.write(h_template)

    with open(c_file, "w") as f:
        print(f"writing version {major}.{minor}.{patch} to {c_file}")
        f.write(c_template)


def generate_source_files() -> None:
    version: str = _package_version_str()
    major, minor, patch = _package_version_from_str(version)
    _generate_c_files(major, minor, patch)
