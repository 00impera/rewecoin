#!/usr/bin/env python3
"""Usage: rewe_align.py check|apply  (run from repo root)"""
import re, sys

CP = "src/chainparams.cpp"
RD = "rust-patches/zcash_protocol/src/constants/"
NETS = {"main": ("CMainParams", "mainnet.rs"),
        "test": ("CTestNetParams", "testnet.rs"),
        "regtest": ("CRegTestParams", "regtest.rs")}
UNIFIED = {"main": ("rwu", "rwuview", "rwuivk"),
           "test": ("rwutest", "rwuviewtest", "rwuivktest"),
           "regtest": ("rwuregtest", "rwuviewregtest", "rwuivkregtest")}
B58 = {"PUBKEY_ADDRESS": "B58_PUBKEY_ADDRESS_PREFIX",
       "SCRIPT_ADDRESS": "B58_SCRIPT_ADDRESS_PREFIX",
       "SECRET_KEY": "B58_SECRET_KEY_PREFIX",
       "ZCPAYMENT_ADDRESS": "B58_SPROUT_ADDRESS_PREFIX"}
HRP = {"SAPLING_PAYMENT_ADDRESS": "HRP_SAPLING_PAYMENT_ADDRESS",
       "SAPLING_EXTENDED_FVK": "HRP_SAPLING_EXTENDED_FULL_VIEWING_KEY",
       "SAPLING_EXTENDED_SPEND_KEY": "HRP_SAPLING_EXTENDED_SPENDING_KEY",
       "TEX_ADDRESS": "HRP_TEX_ADDRESS"}

def sections(text):
    ms = list(re.finditer(r'\b(CMainParams|CTestNetParams|CRegTestParams)\(\)\s*\{', text))
    out = {}
    for i, m in enumerate(ms):
        end = ms[i+1].start() if i+1 < len(ms) else len(text)
        out[m.group(1)] = text[m.start():end]
    return out

def cpp_values(sec):
    v = {}
    for k, body in re.findall(r'base58Prefixes\[(\w+)\]\s*=\s*\{([^}]*)\}', sec):
        if k in B58:
            v[B58[k]] = [int(x, 16) for x in body.split(",")]
    for k, s in re.findall(r'bech32(?:m)?HRPs\[(\w+)\]\s*=\s*"([^"]*)"', sec):
        if k in HRP:
            v[HRP[k]] = s
    m = re.search(r'bip44CoinType\s*=\s*(\d+)', sec)
    if m:
        v["COIN_TYPE"] = int(m.group(1))
    return v

def expected():
    text = open(CP).read()
    secs = sections(text)
    exp = {}
    for net, (cls, _) in NETS.items():
        if cls not in secs:
            sys.exit("ERROR: sectiune lipsa in chainparams: " + cls)
        v = cpp_values(secs[cls])
        a, f, i = UNIFIED[net]
        for h in (a, f, i):
            assert len(h) <= 16, "HRP Unified > 16 bytes: " + h   # limita F4Jumble (ZIP 316)
        v["HRP_UNIFIED_ADDRESS"], v["HRP_UNIFIED_FVK"], v["HRP_UNIFIED_IVK"] = a, f, i
        exp[net] = v
    return exp

def fmt(val):
    if isinstance(val, list):
        return "[" + ", ".join("0x%02x" % b for b in val) + "]"
    return val

def rust_get(text, name):
    m = re.search(r'pub const %s: &str = "([^"]*)";' % name, text)
    if m: return m.group(1)
    m = re.search(r'pub const %s: \[u8; \d+\] = \[([^\]]*)\];' % name, text)
    if m: return [int(x, 16) for x in m.group(1).split(",")]
    m = re.search(r'pub const %s: u32 = (\d+);' % name, text)
    if m: return int(m.group(1))
    return None

def rust_set(text, name, val):
    if isinstance(val, str):
        pat, rep = r'(pub const %s: &str = ")[^"]*(";)' % name, lambda m: m.group(1)+val+m.group(2)
    elif isinstance(val, list):
        pat, rep = r'(pub const %s: \[u8; \d+\] = )\[[^\]]*\](;)' % name, lambda m: m.group(1)+fmt(val)+m.group(2)
    else:
        pat, rep = r'(pub const %s: u32 = )\d+(;)' % name, lambda m: m.group(1)+str(val)+m.group(2)
    return re.subn(pat, rep, text)

def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else "check"
    exp = expected()
    diffs, new_texts, errors = [], {}, []
    for net, (_, rf) in NETS.items():
        path = RD + rf
        text = open(path).read()
        for name, want in exp[net].items():
            have = rust_get(text, name)
            if have is None:
                errors.append("%s: constanta %s nu exista" % (rf, name)); continue
            if have != want:
                diffs.append((rf, name, fmt(have) if isinstance(have, list) else have,
                              fmt(want) if isinstance(want, list) else want))
                if mode == "apply":
                    text, n = rust_set(text, name, want)
                    if n != 1: errors.append("%s: %s ancora gasita de %d ori" % (rf, name, n))
        new_texts[path] = text
    for d in diffs:
        print("DIFF  %-12s %-40s rust=%s  ->  %s" % d)
    for e in errors: print("EROARE", e)
    if mode == "check":
        print("\n%d diferente, %d erori" % (len(diffs), len(errors)))
        sys.exit(1 if diffs or errors else 0)
    if errors:
        print("Nimic nu a fost scris."); sys.exit(1)
    for p, t in new_texts.items(): open(p, "w").write(t)
    vp = "qa/zcash/wallet-builder/lib/verification.py"
    try:
        s = open(vp).read()
        if 'addr.startswith("zs")' in s:
            s = s.replace('addr.startswith("zs")', 'addr.startswith("rws")')
            s = s.replace('"ztestsapling"', '"rwtestsapling"')
            open(vp, "w").write(s); print("verification.py actualizat")
    except FileNotFoundError:
        print("AVERTISMENT: verification.py lipsa")
    print("OK: %d valori scrise" % len(diffs))

main()
