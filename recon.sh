#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_PATH="${SCRIPT_DIR}/bin/ad_scanner"
SRC_PATH="${SCRIPT_DIR}/src/scanner.c"

CYAN='\e[36m'
GREEN='\e[32m'
YELLOW='\e[33m'
RED='\e[31m'
BOLD='\e[1m'
NC='\e[0m'

compile_c() {
	if [[ ! -f "$BIN_PATH" ]] || [[ "$SRC_PATH" -nt "$BIN_PATH" ]]; then
		echo -e "${YELLOW}[*] Compiling C engine...${NC}"
		mkdir -p "${SCRIPT_DIR}/bin"
		if gcc -O2 "$SRC_PATH" -o "$BIN_PATH"; then
			echo -e "${GREEN}[+] Compilation successful!${NC}"
			else
				echo -e "${RED}[!] Compilation failed. Check gcc and source code.${NC}"
				exit 1
				fi
				fi
}

show_banner() {
	echo -e "${CYAN}"
	cat << 'EOF'
	___    ____     ____
	/   |  / __ \   / __ \___  _________  ____
	/ /| | / / / /  / /_/ / _ \/ ___/ __ \/ __ \
	/ ___ |/ /_/ /  / _, _/  __/ /__/ /_/ / / / /
	/_/  |_/_____/  /_/ |_|\___/\___/\____/_/ /_/
EOF
	echo -e "${NC}"
	echo -e "  \e[2mBash Wrapper + C Engine for AD Recon\e[0m\n"
}

main_menu() {
while true; do
	echo -e "${CYAN}════════════════════════════════════════════════════════${NC}"
	echo -e "${CYAN}                       MAIN MENU${NC}"
	echo -e "${CYAN}════════════════════════════════════════════════════════${NC}"
	echo -e "  ${GREEN}1${NC}) Scan host with Nmap (-sC -sV) [C Engine]"
	echo -e "  ${GREEN}2${NC}) Exit"
	echo ""
	read -rp "$(echo -e "${YELLOW}Choose option [1-2]: ${NC}")" choice
	echo ""
	
	case "$choice" in
	1)
	read -rp "$(echo -e "${YELLOW}Target (IP or subnet): ${NC}")" target
	[[ -z "$target" ]] && { echo -e "${RED}[!] Empty target${NC}"; continue; }
	
	mkdir -p "${SCRIPT_DIR}/out"
	local log_file="${SCRIPT_DIR}/out/report_${target//\//_}_$(date +%H%M%S).txt"
	
	"$BIN_PATH" "$target" | tee "$log_file"
	
	echo -e "\n${GREEN}[+] Отчёт сохранен в: ${BOLD}${log_file}${NC}"
	;;
	2)
	echo -e "${GREEN}[+] Goodbye${NC}"
	exit 0
	;;
	*)
	echo -e "${RED}[!] Invalid option${NC}"
	;;
	esac
	
	echo ""
	read -rp "$(echo -e "\e[2mPress Enter to continue...\e[0m")" _
	clear
	show_banner
	done
}

clear
show_banner
compile_c
main_menu
