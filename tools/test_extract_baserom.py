"""ISO reader checks with a synthetic executable; contains no game bytes."""
import hashlib
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import extract_baserom as extract


def directory_record(name, sector, size, flags=0):
    length = (33 + len(name) + 1) & ~1
    record = bytearray(length)
    record[0] = length
    struct.pack_into("<I", record, 2, sector)
    struct.pack_into(">I", record, 6, sector)
    struct.pack_into("<I", record, 10, size)
    struct.pack_into(">I", record, 14, size)
    record[25] = flags
    record[32] = len(name)
    record[33:33 + len(name)] = name
    return record


def fixture(path, filename=b"SCUS_974.65;1"):
    data = bytes(1692216)
    iso = bytearray(22 * extract.SECTOR + len(data))
    descriptor = bytearray(extract.SECTOR)
    descriptor[:7] = b"\x01CD001\x01"
    descriptor[128:132] = struct.pack("<H", extract.SECTOR) + struct.pack(">H", extract.SECTOR)
    root = directory_record(b"\0", 20, 2 * extract.SECTOR, 2)
    descriptor[156:156 + len(root)] = root
    iso[16 * extract.SECTOR:17 * extract.SECTOR] = descriptor
    # Put the executable record after a zero-padded directory sector.
    entry = directory_record(filename, 22, len(data))
    iso[21 * extract.SECTOR:21 * extract.SECTOR + len(entry)] = entry
    path.write_bytes(iso)
    return data


class ISOExtractionTests(unittest.TestCase):
    def setUp(self):
        directory = extract.ROOT / "build/tests"
        directory.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=directory)
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / "synthetic.iso"

    def test_versioned_name_after_sector_padding(self):
        data = fixture(self.path)
        with patch.object(extract, "EXPECTED_SHA1", hashlib.sha1(data).hexdigest()):
            self.assertEqual(extract.extract(self.path), data)

    def test_wrong_game_rejected(self):
        fixture(self.path, b"OTHER.ELF;1")
        with self.assertRaisesRegex(ValueError, "not found"):
            extract.extract(self.path)

    def test_unsupported_hash_rejected(self):
        fixture(self.path)
        with self.assertRaisesRegex(ValueError, "SHA-1"):
            extract.extract(self.path)

    def test_truncated_executable_rejected(self):
        fixture(self.path)
        with self.path.open("r+b") as stream:
            stream.truncate(23 * extract.SECTOR)
        with self.assertRaisesRegex(ValueError, "truncated"):
            extract.extract(self.path)

    def test_inconsistent_extent_rejected(self):
        fixture(self.path)
        with self.path.open("r+b") as stream:
            stream.seek(21 * extract.SECTOR + 6)
            stream.write(struct.pack(">I", 23))
        with self.assertRaisesRegex(ValueError, "inconsistent"):
            extract.extract(self.path)


if __name__ == "__main__":
    unittest.main()
