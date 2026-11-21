# FlyOver

**Active Directory recon, wrapped in Bash, parsed in C.**

`FlyOver` is a small recon tool for Active Directory labs: a Bash interface drives an Nmap scan, and a C engine parses the Nmap output into a compact list of hosts, open ports and the attack vectors worth trying next — Kerberoasting, AS-REP Roasting, SMB relay, null sessions, WinRM, MSSQL and the rest.

```text
[*] C-Engine: Scanning 10.10.10.10 ...

════════════════════════════════════════════════════════
  C-Engine: Analysis and potential attack vectors
════════════════════════════════════════════════════════

▶ Nmap scan report for winterfell.north.sevenkingdoms.local (10.10.10.10)
  53/tcp   open  domain        Simple DNS Plus
  88/tcp   open  kerberos-sec  Microsoft Windows Kerberos
  135/tcp  open  msrpc         Microsoft Windows RPC
  389/tcp  open  ldap          Microsoft Windows Active Directory LDAP
  445/tcp  open  microsoft-ds
  5985/tcp open  http          Microsoft HTTPAPI httpd 2.0
  [!] Potential attack vectors:
    • DNS (53): zone transfer (AXFR), DNS tunneling
    • Kerberos (88): AS-REP Roasting, Kerberoasting, password spraying
    • MSRPC (135): enum4linux, rpcclient, null session
    • LDAP (389): anonymous bind, user enum, BloodHound
    • SMB (445): null session, shares enum, NTLM relay, signing check
    • WinRM (5985): password spraying, pass-the-hash (evil-winrm)
```

## Why two languages

Bash is good at the thing the user touches — a menu, colours, prompts, saving a report. It is bad at reading a 3000-line Nmap file line by line. C is the opposite. The split keeps the interface easy to edit and the parsing fast and dependency-free: no Python, no `jq`, no `awk` gymnastics.

The C engine is compiled on first run from `src/scanner.c` — there is no binary to trust in the repository.

## Features

- Interactive menu: target in, report out.
- Nmap `-sC -sV` under the hood, output kept in `out/` as a timestamped report.
- Parses multiple hosts per scan, groups ports per host, prints one vector list per host.
- Colour-coded output; unknown ports are explicitly flagged for manual review.
- Strict target validation — the argument reaches a shell, so only `[A-Za-z0-9._/,-]` is accepted.
- Temporary scan file is created with `mkstemp()` and removed after parsing.

## Requirements

| Tool | Why |
|---|---|
| `bash` 4+ | the wrapper |
| `nmap` | the actual scanning |
| `gcc` | builds the C engine on first run (or `make`) |

Linux only: the engine relies on `/tmp`, `mkstemp()` and `system()` shell semantics.

## Install

```bash
git clone https://github.com/McMeelon/flyover.git
cd flyover
chmod +x install.sh
./install.sh
```

The installer checks dependencies, builds nothing yet, and symlinks `ad-recon` into `~/.local/bin`. It warns if that directory is missing from `PATH`.

Run it:

```bash
ad-recon
```

Or without installing:

```bash
./recon.sh
```

Uninstall:

```bash
./uninstall.sh
```

## Usage

1. Pick `1) Scan host with Nmap`.
2. Enter a target: `10.10.10.10`, `dc.lab.local`, `10.10.10.0/24`, or a list `10.10.10.1,10.10.10.2`.
3. Read the report on screen; it is also saved to `out/report_<target>_<HHMMSS>.txt`.

Scanning a subnet takes a while — `-sC -sV` runs default scripts and version detection on every open port.

## How it works

```text
recon.sh ──► bin/ad_scanner <target>
                │
                ├─ validate target (allowlist of characters)
                ├─ mkstemp() ──► /tmp/flyover_scan_XXXXXX
                ├─ system("nmap -sC -sV -T4 <target> -oN <tmp>")
                ├─ parse: "Nmap scan report for" starts a host block
                │          "<port>/tcp open ..." fills the port list
                ├─ map port ──► attack vectors
                └─ unlink(<tmp>)
        ◄── stdout ──► tee ──► out/report_<target>_<time>.txt
```

## Project layout

```text
flyover/
├── recon.sh          # menu, colours, report saving
├── install.sh        # dependency check + symlink to ~/.local/bin
├── uninstall.sh      # removes the symlink
├── src/scanner.c     # Nmap output parser and vector mapper
├── bin/              # compiled engine lands here (gitignored)
├── out/              # timestamped reports (gitignored)
└── Makefile          # make / make run / make clean
```

## Build manually

```bash
make          # gcc -O2 -Wall -Wextra -o bin/ad_scanner src/scanner.c
make clean
```

## Limitations

- Parses Nmap's **English** output only — a localised Nmap will produce an empty report.
- Recognises the ports listed in `src/scanner.c`; everything else is reported as "requires manual investigation" on purpose.
- No IPv6 host ranges.
- No machine-readable output (JSON/CSV) yet.

## Roadmap

- [ ] JSON output for piping into other tools
- [ ] Flag risky finds automatically: `signing:False`, anonymous LDAP bind, MS17-010
- [ ] Optional `--no-nmap` mode to parse an existing `.nmap` file
- [ ] ShellCheck in CI

## Legal

Use only against systems you own or have written permission to test. Scanning networks you do not control is illegal in most jurisdictions. The tool is deliberately conservative — it does not exploit anything, only points at what is worth checking.

## License

No license file: the source is published for reading and learning. If you want to reuse it in your own project, ask first.
