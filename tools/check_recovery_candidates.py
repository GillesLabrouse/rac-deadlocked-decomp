#!/usr/bin/env python3
"""Reject already recovered canonical functions, including library aliases.

Run before drafting or compiling a matching recovery batch. This inventory
guard does not establish ownership, bounds, ABI, relocations or exact matching.
Those checks still require the actual reference for the intended level.
"""
import argparse
import json

import gen_progress_report as progress


def inventory(names, owned=()):
    functions = progress.load_functions()
    sources = {}
    for path, symbols in progress.source_functions().items():
        for symbol in symbols:
            sources.setdefault(symbol, []).append(path)
    reserved = set(owned)
    result = []
    for name in names:
        reason = ("unknown_canonical_name" if name not in functions else
                  "already_recovered" if name in sources else
                  "owned_by_another_batch" if name in reserved else "available")
        result.append({"name": name, "status": reason,
                       "sources": sources.get(name, [])})
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("names", nargs="+", help="canonical func_XXXXXXXX names")
    parser.add_argument("--owned", action="append", default=[],
                        help="canonical name reserved by another batch; repeatable")
    args = parser.parse_args()
    result = inventory(args.names, args.owned)
    print(json.dumps(result, indent=2))
    return int(any(item["status"] != "available" for item in result))


if __name__ == "__main__":
    raise SystemExit(main())
