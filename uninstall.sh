#!/usr/bin/env bash
set -euo pipefail

BIN_DIR="${HOME}/.local/bin"
LINK="${BIN_DIR}/ad-recon"

if [[ -L "$LINK" ]]; then
	rm "$LINK"
	echo "[+] Removed symlink: $LINK"
else
	echo "[*] No symlink found at $LINK"
fi
