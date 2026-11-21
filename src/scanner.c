#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

void print_vectors(int port) {
	switch (port) {
		case 21:  printf("    \033[33m• FTP (21):\033[0m anonymous login, brute force, outdated versions\n"); break;
		case 22:  printf("    \033[33m• SSH (22):\033[0m brute force, weak keys, outdated versions\n"); break;
		case 53:  printf("    \033[33m• DNS (53):\033[0m zone transfer (AXFR), DNS tunneling\n"); break;
		case 80:
		case 443: printf("    \033[33m• HTTP/HTTPS (%d):\033[0m gobuster, nikto, Burp, security headers\n", port); break;
		case 88:  printf("    \033[33m• Kerberos (88):\033[0m AS-REP Roasting, Kerberoasting, password spraying\n"); break;
		case 135: printf("    \033[33m• MSRPC (135):\033[0m enum4linux, rpcclient, null session\n"); break;
		case 389:
		case 636: printf("    \033[33m• LDAP (%d):\033[0m anonymous bind, user enum, BloodHound\n", port); break;
		case 445: printf("    \033[33m• SMB (445):\033[0m null session, shares enum, NTLM relay, signing check\n"); break;
		case 1433:printf("    \033[33m• MSSQL (1433):\033[0m password spraying, xp_cmdshell, trusted links\n"); break;
		case 3389:printf("    \033[33m• RDP (3389):\033[0m password spraying, BlueKeep (CVE-2019-0708), RDP hijacking\n"); break;
		case 5985:
		case 5986:printf("    \033[33m• WinRM (%d):\033[0m password spraying, pass-the-hash (evil-winrm)\n", port); break;
		default:  printf("    \033[90m• Port %d:\033[0m requires manual investigation\n", port); break;
	}
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <target_ip | path_to_nmap_file>\n", argv[0]);
        return 1;
    }

    char *target = argv[1];
    char outfile[256];
    struct stat st;

    
    if (stat(target, &st) == 0 && (st.st_mode & S_IFREG)) {
        // Это файл! Используем его.
        strncpy(outfile, target, sizeof(outfile)-1);
        printf("\033[33m[*] Parsing existing Nmap file: %s\033[0m\n", outfile);
    } else {
        // Это IP. Запускаем Nmap.
        snprintf(outfile, sizeof(outfile), "/tmp/ad_recon_scan.txt");
        char cmd[512];
        printf("\033[33m[*] C-Engine: Scanning %s ...\033[0m\n", target);
        snprintf(cmd, sizeof(cmd), "nmap -sC -sV -T4 %s -oN %s > /dev/null 2>&1", target, outfile);
        system(cmd);
    }

    
    FILE *fp = fopen(outfile, "r");
    if (!fp) {
        printf("\033[31m[!] Error: Could not open scan results (%s).\033[0m\n", outfile);
        return 1;
    }

    printf("\n\033[36m════════════════════════════════════════════════════════\033[0m\n");
    printf("\033[36m  C-Engine: Analysis and potential attack vectors\033[0m\n");
    printf("\033[36m════════════════════════════════════════════════════════\033[0m\n");

    char line[1024];
    int ports[100];
    int port_count = 0;
    int current_port = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Nmap scan report for", 20) == 0) {
            if (port_count > 0) {
                printf("  \033[33m\033[1m[!] Potential attack vectors:\033[0m\n");
                for (int i = 0; i < port_count; i++) {
                    print_vectors(ports[i]);
                }
                port_count = 0;
            }
            printf("\n\033[32m\033[1m▶ %s\033[0m", line);
        }
        else if (sscanf(line, "%d/tcp", &current_port) == 1 || sscanf(line, "%d/udp", &current_port) == 1) {
            printf("  \033[32m%s\033[0m", line);
            ports[port_count++] = current_port;
        }
       
        else if (strstr(line, "signing enabled but not required") != NULL) {
            printf("  \033[31m\033[1m[!] VULN DETECTED: SMB Signing is DISABLED (NTLM Relay possible!)\033[0m\n");
        }
    }

    if (port_count > 0) {
        printf("  \033[33m\033[1m[!] Potential attack vectors:\033[0m\n");
        for (int i = 0; i < port_count; i++) {
            print_vectors(ports[i]);
        }
    }

    fclose(fp);
    return 0;
}
