#!/usr/bin/env python3
import argparse
import os
import re
import subprocess
import sys

ADDR_RE = re.compile(r'0x[0-9a-fA-F]+')
FRAME_RE = re.compile(r'^(?P<func>.+?) at (?P<file>.+?):(?P<line>\d+)(?: \(discriminator \d+\))?\s*$')

def format_frame(s):
    if s == '?? at ??:?' or s == '?? at ??:0':
        return '??(??)'
    m = FRAME_RE.match(s)
    if not m:
        return re.sub(r'\s+\(discriminator \d+\)\s*$', '', s)
    func = m.group('func').split('(')[0]
    base = os.path.basename(m.group('file'))
    return f"{func}({base}:{m.group('line')})"

def resolve_many(addr2line, exe, addrs):
    if not addrs:
        return {}
    cmd = [addr2line, '-e', exe, '-f', '-p', '-C'] + list(addrs)
    try:
        p = subprocess.run(cmd, check=True, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE, text=True, errors='replace')
        lines = p.stdout.splitlines()
        if len(lines) == len(addrs):
            return {a: b for a, b in zip(addrs, lines)}
    except Exception:
        pass
    out = {}
    for a in addrs:
        try:
            p = subprocess.run([addr2line, '-e', exe, '-f', '-p', '-C', a],
                               check=True, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, text=True, errors='replace')
            lines = p.stdout.splitlines()
            out[a] = lines[0] if lines else a
        except Exception:
            out[a] = a
    return out

def main():
    ap = argparse.ArgumentParser(description='symbolize rtlog backtraces')
    ap.add_argument('exe')
    ap.add_argument('rtlog')
    ap.add_argument('-o', '--output')
    ap.add_argument('--addr2line', default='addr2line')
    args = ap.parse_args()
    output = args.output if args.output else args.rtlog + '.sym'

    with open(args.rtlog, 'r', errors='replace') as f:
        lines = f.readlines()

    seen = []
    seen_set = set()
    for line in lines:
        idx = line.find(' backtrace: ')
        if idx == -1:
            continue
        rest = line[idx + len(' backtrace: '):]
        for a in ADDR_RE.findall(rest):
            if a not in seen_set:
                seen.append(a)
                seen_set.add(a)

    symbols = resolve_many(args.addr2line, args.exe, seen)

    with open(output, 'w') as f:
        for line in lines:
            idx = line.find(' backtrace: ')
            if idx == -1:
                f.write(line)
                continue
            prefix = line[:idx + len(' backtrace: ')]
            rest = line[idx + len(' backtrace: '):]
            addrs = ADDR_RE.findall(rest)
            if not addrs:
                f.write(line)
                continue
            symlist = [format_frame(symbols.get(a, a)) for a in addrs]
            tail = ADDR_RE.sub('', rest)
            tail = ' '.join(tail.split())
            if tail:
                f.write(prefix + ' <- '.join(symlist) + ' ' + tail + '\n')
            else:
                f.write(prefix + ' <- '.join(symlist) + '\n')

if __name__ == '__main__':
    main()
