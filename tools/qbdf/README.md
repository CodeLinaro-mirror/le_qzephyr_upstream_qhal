# BDF File Conversion Tool Documentation

## Overview

BDF tools are used to convert WiFi configuration files between text and binary formats.

## Prerequisites

- Python 3.x
- `NvTpl.txt` template file (must be in current directory)

## Usage

### Binary to Text (bin → txt)

```bash
python BDF_bin2txt.py <directory>
```

**Example:**
```bash
python BDF_bin2txt.py ./bdf_flie
```

**Result:** Generates corresponding `.txt` files in the bdf_file directory

---

### Text to Binary (txt → bin)

```bash
python BDF_txt2elf.py <directory>
```

**Example:**
```bash
python BDF_txt2elf.py ./bdf_file
```

**Result:** Generates corresponding `.bin` files in the bdf_file directory

---

## Typical Workflow
1. bin → txt: python BDF_bin2txt.py ./bdf_file
2. Edit txt files
3. txt → bin: python BDF_txt2bin.py ./bdf_file

## Important Notes

- `NvTpl.txt` must be in current directory (not target directory)
- Backup original files before conversion
- Ensure directory has write permissions

## Common Errors

| Error | Solution |
|-------|----------|
| NvTpl.txt not found | Place NvTpl.txt in current directory |
| _new.txt files exist | Delete all `*_new.txt` files |
| Index out of range | Check if bin file is corrupted |

## File Naming Convention

- `mqm730i.txt` ↔ `mqm730i.bin` 