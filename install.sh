#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="${HOME}/.local/bin"

echo "[*] Installing FlyOver (Bash + C)..."

missing=()
command -v nmap >/dev/null 2>&1 || missing+=("nmap")
command -v gcc >/dev/null 2>&1 || missing+=("gcc")

if [[ ${#missing[@]} -gt 0 ]]; then
	echo "[!] Missing dependencies: ${missing[*]}"
	echo "    Install with: sudo apt update && sudo apt install -y ${missing[*]}"
	exit 1
fi
echo "[+] Dependencies OK (nmap, gcc found)"

chmod +x "${SCRIPT_DIR}/recon.sh" "${SCRIPT_DIR}/uninstall.sh"
mkdir -p "$BIN_DIR" "${SCRIPT_DIR}/bin" "${SCRIPT_DIR}/out"
ln -sf "${SCRIPT_DIR}/recon.sh" "${BIN_DIR}/ad-recon"
echo "[+] Symlink created: ${BIN_DIR}/ad-recon"

# Без этого шага команда ad-recon не найдётся в новом терминале
case ":${PATH}:" in
	*":${BIN_DIR}:"*) ;;
	*)
		echo "[!] ${BIN_DIR} is not in your PATH. Add this line to ~/.bashrc:"
		echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
		;;
esac

echo -e "\n[+] Done! Run 'ad-recon'"
