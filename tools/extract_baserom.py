#!/usr/bin/env python3
"""Extract and verify the supported executable from a user's ISO9660 disc image.

Only the root-directory SCUS_974.65 is read; the sector-addressed game assets
are left in the ISO. No third-party packages or mounted disc are required.
"""
import argparse
import hashlib
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SECTOR = 2048
EXPECTED_SHA1 = "aa91b1c3b9b1a244320c47580b77342ef9856e95"
EXECUTABLE = "SCUS_974.65"


def read_extent(stream, sector, size):
    stream.seek(sector * SECTOR)
    data = stream.read(size)
    if len(data) != size:
        raise ValueError("truncated ISO extent")
    return data


def record_extent(record):
    if len(record) < 34 or record[0] != len(record):
        raise ValueError("invalid ISO9660 directory record")
    sector, size = struct.unpack_from("<I", record, 2)[0], struct.unpack_from("<I", record, 10)[0]
    if sector != struct.unpack_from(">I", record, 6)[0] or size != struct.unpack_from(">I", record, 14)[0]:
        raise ValueError("inconsistent ISO9660 extent")
    if record[1] or record[26] or record[27] or record[25] & 0x80:
        raise ValueError("extended, interleaved or multi-extent files are unsupported")
    return sector, size


def read_executable(iso):
    """Read SCUS_974.65 without identifying the build (also used for rebuilt ISOs)."""
    with iso.open("rb") as stream:
        descriptor = None
        for sector in range(16, 80):
            block = read_extent(stream, sector, SECTOR)
            if block[1:7] != b"CD001\x01":
                raise ValueError("not a supported ISO9660 disc image")
            if block[0] == 1:
                descriptor = block
                break
            if block[0] == 255:
                break
        if descriptor is None:
            raise ValueError("ISO9660 primary volume descriptor not found")
        if descriptor[128:132] != struct.pack("<H", SECTOR) + struct.pack(">H", SECTOR):
            raise ValueError("expected 2048-byte ISO sectors")
        root = descriptor[156:156 + descriptor[156]]
        directory = read_extent(stream, *record_extent(root))
        pos = 0
        while pos < len(directory):
            length = directory[pos]
            if not length:
                pos = (pos // SECTOR + 1) * SECTOR
                continue
            if pos + length > len(directory) or pos % SECTOR + length > SECTOR:
                raise ValueError("truncated ISO9660 directory record")
            record = directory[pos:pos + length]
            extent = record_extent(record)
            name_size = record[32]
            if 33 + name_size > length:
                raise ValueError("truncated ISO9660 file name")
            name = record[33:33 + name_size].split(b";", 1)[0]
            if name == EXECUTABLE.encode() and not record[25] & 2:
                if not 0 < extent[1] <= 16 * 1024 * 1024:
                    raise ValueError("invalid executable size: %d" % extent[1])
                data = read_extent(stream, *extent)
                return data
            pos += length
    raise ValueError("%s not found: supply the NTSC-U v1.00 disc" % EXECUTABLE)


def extract(iso):
    data = read_executable(iso)
    if len(data) != 1692216:
        raise ValueError("unsupported executable size: %d" % len(data))
    digest = hashlib.sha1(data).hexdigest()
    if digest != EXPECTED_SHA1:
        raise ValueError("unsupported executable SHA-1: %s (expected %s)" % (digest, EXPECTED_SHA1))
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", type=Path)
    args = parser.parse_args()
    try:
        data = extract(args.iso)
        output = ROOT / "baserom" / EXECUTABLE
        output.parent.mkdir(exist_ok=True)
        output.write_bytes(data)
    except (OSError, ValueError) as error:
        parser.exit(1, "%s\n" % error)
    print("Verified %s (%s); wrote %s" % (EXECUTABLE, EXPECTED_SHA1, output))


if __name__ == "__main__":
    main()
