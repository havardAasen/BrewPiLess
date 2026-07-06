from pathlib import Path
import shutil

# pylint: disable=E0602
Import("env")  # noqa


language = "english"

#for key, val in env.items():
    #print(f"{key}: {val}")

for define in env["CPPDEFINES"]:
    if isinstance(define, tuple) and define[0] == "WebPageLanguage":
        language = define[1]
        break

print(f"Selected language: {language}")

src_dir = Path("frontend", "dist", language)
dst_dir = Path("data", "www")

if dst_dir.exists():
    shutil.rmtree(dst_dir)

if not src_dir.exists():
    raise RuntimeError(f"Failed to find folder {src_dir}")

print(f"Copying language folder {src_dir}")
shutil.copytree(src_dir, dst_dir, ignore=shutil.ignore_patterns("BPL*.htm"))
