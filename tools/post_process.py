# -*- coding: utf-8 -*-
# Qualcomm Internal - post build processing for ELF images (hex/strip/bin/hashed)
import argparse
import os
import shutil
import subprocess
import sys


def run(cmd, cwd=None, check=True, desc=None):
    print("\n[RUN] {}{}".format(f"[{desc}] " if desc else "", " ".join(cmd)))
    sys.stdout.flush()
    proc = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if proc.stdout:
        print(proc.stdout.rstrip())
    if proc.returncode != 0:
        print(proc.stderr.rstrip())
        if check:
            raise RuntimeError(f"Command failed (rc={proc.returncode}): {' '.join(cmd)}")
    return proc


def _has_path(s: str) -> bool:
    """Return True if 's' looks like a path (absolute, or contains a path separator).
    Handles both forward and backward slashes on Windows."""
    return os.path.isabs(s) or (os.path.sep in s) or (os.path.altsep and os.path.altsep in s)

def find_tool(prefix: str, name: str) -> str:
    """
    Resolve a binutils tool from a given prefix and tool name, cross-platform.

    Supports:
      - Bare prefix (e.g., 'arm-zephyr-eabi'): search via PATH.
      - Absolute prefix (e.g., 'C:/.../arm-zephyr-eabi/bin/arm-zephyr-eabi'): search in that directory.
    Returns:
      Absolute path to the resolved tool. On Windows, a valid extension from PATHEXT (e.g. .exe) is applied.
    Raises:
      FileNotFoundError if the tool cannot be located.
    """
    tool_base = f"{os.path.basename(prefix)}-{name}"  # Keep only the filename portion (no directory)

    if _has_path(prefix):
        tool_dir = os.path.dirname(prefix)

        # 1) Prefer shutil.which with a restricted search path so Windows applies PATHEXT (adds .exe, etc.)
        found = shutil.which(tool_base, path=tool_dir)
        if found:
            return os.path.abspath(found)

        # 2) Fallback: direct path; on Windows, try PATHEXT manually if needed
        direct = os.path.join(tool_dir, tool_base)
        if os.path.isfile(direct):
            return os.path.abspath(direct)

        if os.name == "nt":
            pathexts = os.environ.get("PATHEXT", ".EXE;.BAT;.CMD;.COM").split(";")
            for ext in pathexts:
                ext = ext.strip()
                if not ext:
                    continue
                if not ext.startswith("."):
                    ext = "." + ext
                cand = direct + ext
                if os.path.isfile(cand):
                    return os.path.abspath(cand)

    # 3) Fallback: search the system PATH using the full "prefix-name" form
    cand = f"{prefix}-{name}"
    found = shutil.which(cand)
    if found:
        return os.path.abspath(found)

    search_dir = os.path.dirname(prefix) if _has_path(prefix) else "<PATH>"
    raise FileNotFoundError(
        "Unable to locate tool.\n"
        f"  Tried: '{cand}'\n"
        f"  Search dir: {search_dir}\n"
        "Hint: Add the SDK '.../arm-zephyr-eabi/bin' directory to PATH, or pass an absolute "
        "tool prefix such as 'C:/.../arm-zephyr-eabi/bin/arm-zephyr-eabi'."
    )



def main():
    ap = argparse.ArgumentParser(description="ELF post-processing: hex/size/strip/bin/hashed")
    ap.add_argument("--elf", required=True, help="Path to input ELF file")
    ap.add_argument("--tool-prefix", default="arm-zephyr-eabi",
                    help="Tool prefix (default: arm-zephyr-eabi). Use arm-none-eabi for old toolchain")
    ap.add_argument("--gap-fill", default=None,
                    help="When producing BIN, pass objcopy --gap-fill <val> (e.g. 0xFF)")
    ap.add_argument("--hash-script", default=None,
                    help="Path to tools/sechash/createxbl.py; if absent, hashed step is skipped")
    ap.add_argument("--hashed-out", default=None,
                    help="Output path for hashed ELF (default: <elf_dir>/<elf_base>_HASHED.elf)")
    ap.add_argument("--check-noinit", action="store_true",
                    help="Check and skip segments containing noinit sections when hashing (for app images)")
    ap.add_argument("--copy-nvm-prog", action="store_true",
                    help="If base name equals FERMION_NVM_PROGRAMMER, copy ELF to tools/nvm_programmer/bin")
    args = ap.parse_args()

    elf = os.path.abspath(args.elf)
    if not os.path.exists(elf):
        print(f"[ERR] ELF not found: {elf}")
        sys.exit(2)

    elf_dir = os.path.dirname(elf)
    elf_base = os.path.splitext(os.path.basename(elf))[0]
    os.makedirs(elf_dir, exist_ok=True)

    objcopy = find_tool(args.tool_prefix, "objcopy")
    size    = find_tool(args.tool_prefix, "size")

    # 1) HEX
    hex_out = os.path.join(elf_dir, f"{elf_base}.hex")
    run([objcopy, "-O", "ihex", elf, hex_out], desc="Generate .hex")

    # 2) size (berkeley)
    run([size, "--format=berkeley", "--totals", elf], desc="Print size (berkeley)")

    # 3) STRIPPED ELF
    stripped_out = os.path.join(elf_dir, f"{elf_base}_STRIPPED.elf")
    run([objcopy, elf, "--strip-all", stripped_out], desc="Strip ELF")

    # 4) BIN
    bin_out = os.path.join(elf_dir, f"{elf_base}.bin")
    cmd_bin = [objcopy, elf, "-O", "binary", bin_out]
    if args.gap_fill:
        cmd_bin = [objcopy, "-O", "binary", "--gap-fill", args.gap_fill, elf, bin_out]
    run(cmd_bin, desc="Generate .bin")

    # 5) optional copy for NVM programmer (保留你原来的特例)
    if args.copy_nvm_prog and elf_base == "FERMION_NVM_PROGRAMMER":
        current_path = os.path.dirname(os.path.abspath(__file__))
        tools_bin_path = os.path.join(current_path, "../../../tools/nvm_programmer/bin")
        os.makedirs(tools_bin_path, exist_ok=True)
        shutil.copy2(elf, os.path.join(tools_bin_path, f"{elf_base}.elf"))
        print(f"[INFO] Copied to {tools_bin_path}")

    # 6) Hashed ELF via createxbl.py
    if args.hash_script:
        hash_script = os.path.abspath(args.hash_script)
        if os.path.isfile(hash_script):
            hashed_out = args.hashed_out or os.path.join(elf_dir, f"{elf_base}_HASHED.elf")
            # createxbl.py: python createxbl.py -f <elf> -a32 -o <out> [-i]
            cmd_hash = [sys.executable, hash_script, "-f", elf, "-a32", "-o", hashed_out]
            if args.check_noinit:
                cmd_hash.append("-i")
            run(cmd_hash, desc="Generate hashed ELF (createxbl.py)")
        else:
            print(f"[WARN] hash-script not found: {hash_script} -> skip hashed ELF")
    else:
        print("[INFO] hash-script not provided -> skip hashed ELF")

    print("\n[OK] Post-processing finished.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as e:
        print(f"[FATAL] {e}")
        sys.exit(1)
