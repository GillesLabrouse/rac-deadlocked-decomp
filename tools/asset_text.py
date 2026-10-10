"""Read the small structured text grammar observed in local Wrench exports.

This parser is authored here and does not import any Wrench source. It handles
named blocks, attributes, arrays, numbers, strings and booleans. Files are input
data, never executable instructions.
"""

import json
import math
import re
from dataclasses import dataclass, field
from pathlib import Path

TOKEN = re.compile(r'''(?P<space>\s+|//[^\n]*|/\*.*?\*/)|(?P<string>"(?:\\.|[^"\\])*")|(?P<number>[+-]?(?:0[xX][0-9a-fA-F]+|(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?))|(?P<name>[A-Za-z_][\w.\-]*)|(?P<symbol>[{}\[\]:])|(?P<invalid>.)''', re.S)


@dataclass
class Node:
    kind: str
    name: str | None = None
    attributes: dict = field(default_factory=dict)
    children: list = field(default_factory=list)


class Parser:
    def __init__(self, text):
        self.tokens = [(m.lastgroup, m.group()) for m in TOKEN.finditer(text) if m.lastgroup != "space"]
        self.cursor = 0

    def peek(self):
        return self.tokens[self.cursor][1] if self.cursor < len(self.tokens) else None

    def take(self):
        if self.cursor >= len(self.tokens):
            raise ValueError("Unexpected end of asset text")
        token = self.tokens[self.cursor]
        self.cursor += 1
        if token[0] == "invalid":
            raise ValueError("Invalid asset token: " + token[1])
        return token

    def expect(self, token):
        if self.take()[1] != token:
            raise ValueError("Expected asset token: " + token)

    def value(self):
        if self.peek() == "[":
            self.take()
            values = []
            while self.peek() != "]":
                values.append(self.value())
            self.expect("]")
            return values
        kind, text = self.take()
        if kind == "string":
            return json.loads(text)
        if kind == "number":
            if "x" in text.lower():
                return int(text, 16)
            if "." in text or "e" in text.lower():
                value = float(text)
                if not math.isfinite(value):
                    raise ValueError("Non-finite asset number")
                return value
            return int(text, 10)
        if kind == "name" and text in ("true", "false"):
            return text == "true"
        raise ValueError("Unsupported asset value: " + text)

    def node(self, kind=None):
        if kind is None:
            category, kind = self.take()
            if category != "name":
                raise ValueError("Expected an asset block type")
        name = None
        if self.peek() != "{":
            category, name = self.take()
            if category not in ("name", "number", "string"):
                raise ValueError("Invalid asset block name")
            if category == "string":
                name = json.loads(name)
        self.expect("{")
        node = Node(kind, name)
        while self.peek() != "}":
            category, key = self.take()
            if category != "name":
                raise ValueError("Expected an asset attribute or child block")
            if self.peek() == ":":
                self.take()
                if key in node.attributes:
                    raise ValueError("Duplicate asset attribute: " + key)
                node.attributes[key] = self.value()
            else:
                node.children.append(self.node(key))
        self.expect("}")
        return node

    def parse(self):
        roots = []
        metadata = Node("file_attributes")
        while self.peek() is not None:
            category, key = self.take()
            if category != "name":
                raise ValueError("Expected an asset block or file attribute")
            if self.peek() == ":":
                self.take()
                if key in metadata.attributes:
                    raise ValueError("Duplicate file attribute: " + key)
                metadata.attributes[key] = self.value()
            else:
                roots.append(self.node(key))
        if metadata.attributes:
            roots.append(metadata)
        return roots


def read(path):
    return Parser(Path(path).read_text(encoding="utf-8")).parse()
