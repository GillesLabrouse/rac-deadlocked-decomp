#!/usr/bin/env python3
"""Reject already recovered canonical functions, including library aliases.

Run before drafting or compiling a matching recovery batch. This inventory
guard does not establish ownership, bounds, ABI, relocations or exact matching.
Those checks still require the actual reference for the intended level.
"""
import argparse
import json
import re

import gen_progress_report as progress


def native_mentions(name):
    """Find address mentions, not a coverage or cross-level equivalence claim."""
    suffix = re.search(r"([0-9A-Fa-f]{8})$", name)
    if suffix is None:
        return []
    address = suffix.group(1).lstrip("0") or "0"
    pattern = re.compile(r"(?<![0-9a-f])(?:0x)?0*" + address +
                         r"(?![0-9a-f])", re.I)
    mentions = []
    for path in sorted((progress.ROOT / "native").glob("*")):
        if path.suffix not in (".c", ".h"):
            continue
        matches = [{"line": number, "text": line.strip()[:240]}
                   for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1)
                   if pattern.search(line)]
        if matches:
            mentions.append({"path": path.relative_to(progress.ROOT).as_posix(),
                             "matches": matches[:3]})
    return mentions


def inventory(names, owned=(), include_native=False):
    functions = progress.load_functions()
    sources = {}
    for path, symbols in progress.source_functions().items():
        for symbol in symbols:
            sources.setdefault(symbol, []).append(path)
    reserved = set(owned)
    library_hints = {}
    for library in ("libc", "libm", "libgcc"):
        path = progress.ROOT / "config" / (library + ".tsv")
        if not path.exists():
            continue
        for line in path.read_text().splitlines():
            if not line or line.startswith("#"):
                continue
            member, symbol, address, size = line.split("\t")
            library_hints.setdefault(int(address, 16), []).append({
                "library": library, "member": member, "symbol": symbol,
                "size": int(size, 16)})
    result = []
    for name in names:
        function = functions.get(name)
        hints = [] if function is None or function["seg"].startswith("ovl_") else library_hints.get(function["addr"], [])
        reason = ("unknown_canonical_name" if name not in functions else
                  "already_recovered" if name in sources else
                  "owned_by_another_batch" if name in reserved else
                  "library_mapping_requires_review" if hints else "available")
        item = {"name": name, "status": reason, "sources": sources.get(name, [])}
        if hints:
            item["library_hints"] = hints
        if include_native:
            item["native_mentions"] = native_mentions(name)
        result.append(item)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("names", nargs="+", help="canonical func_XXXXXXXX names")
    parser.add_argument("--owned", action="append", default=[],
                        help="canonical name reserved by another batch; repeatable")
    parser.add_argument("--native-evidence", action="store_true",
                        help="also list native address mentions; inspect their actual level and supported domain")
    args = parser.parse_args()
    result = inventory(args.names, args.owned, args.native_evidence)
    print(json.dumps(result, indent=2))
    return int(any(item["status"] != "available" for item in result))


if __name__ == "__main__":
    raise SystemExit(main())
