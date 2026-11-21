#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Цвета для терминала */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

#define MAX_PORTS 100

/*
 * Цель приходит из argv и уходит в system() — поэтому проверяем её строго.
 * Разрешены только символы, из которых состоят IP, hostname, CIDR и списки
 * через запятую. Точка с запятой, пайп, амперсанд, кавычки и прочие
 * метасимволы оболочки сюда не попадают.
 */
static int is_valid_target(const char *s)
{
	if (s == NULL || *s == '\0')
		return 0;

	size_t len = strlen(s);
	if (len > 128)
		return 0;

	for (size_t i = 0; i < len; i++) {
		unsigned char c = (unsigned char)s[i];
		if (isalnum(c) || c == '.' || c == '-' || c == '/' ||
		    c == ',' || c == '_')
			continue;
		return 0;
	}

	return 1;
}

void print_vectors(int port)
{
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

/* Печатает накопленные векторы для предыдущего хоста и обнуляет счётчик */
static void flush_vectors(int *ports, int *count)
{
	if (*count <= 0)
		return;

	printf("  \033[33m" BOLD "[!] Potential attack vectors:" RESET "\n");
	for (int i = 0; i < *count; i++)
		print_vectors(ports[i]);
	*count = 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <target_ip|host|CIDR>\n", argv[0]);
		return 1;
	}

	char *target = argv[1];

	if (!is_valid_target(target)) {
		printf(RED "[!] Invalid target: allowed characters are letters, digits, "
		           "dot, dash, slash, comma and underscore." RESET "\n");
		return 2;
	}

	/* mkstemp даёт уникальное имя во временном каталоге и сразу создаёт файл:
	   предсказуемого пути в /tmp больше нет, гонка и симлинк-атака закрыты */
	char outfile[] = "/tmp/flyover_scan_XXXXXX";
	int fd = mkstemp(outfile);
	if (fd < 0) {
		printf(RED "[!] Error: cannot create a temporary file." RESET "\n");
		return 1;
	}
	close(fd);

	char cmd[512];

	/* 1. Запускаем Nmap */
	printf(YELLOW "[*] C-Engine: Scanning %s ..." RESET "\n", target);
	snprintf(cmd, sizeof(cmd), "nmap -sC -sV -T4 %s -oN %s > /dev/null 2>&1",
	         target, outfile);
	int rc = system(cmd);
	if (rc != 0)
		printf(YELLOW "[!] nmap exited with status %d — results may be incomplete." RESET "\n",
		       rc);

	/* 2. Открываем файл с результатами */
	FILE *fp = fopen(outfile, "r");
	if (!fp) {
		printf(RED "[!] Error: could not open scan results." RESET "\n");
		unlink(outfile);
		return 1;
	}

	printf("\n\033[36m════════════════════════════════════════════════════════\033[0m\n");
	printf("\033[36m  C-Engine: Analysis and potential attack vectors\033[0m\n");
	printf("\033[36m════════════════════════════════════════════════════════\033[0m\n");

	char line[1024];
	int ports[MAX_PORTS];
	int port_count = 0;
	int current_port = 0;

	/* 3. Парсим файл */
	while (fgets(line, sizeof(line), fp)) {
		line[strcspn(line, "\r\n")] = '\0';

		if (strncmp(line, "Nmap scan report for", 20) == 0) {
			/* Новый хост — печатаем векторы предыдущего */
			flush_vectors(ports, &port_count);
			printf("\n" GREEN BOLD "▶ %s" RESET "\n", line);
		} else if (sscanf(line, "%d/tcp", &current_port) == 1 ||
		           sscanf(line, "%d/udp", &current_port) == 1) {
			printf("  " GREEN "%s" RESET "\n", line);
			/* Граница массива: у хоста может быть больше портов, чем MAX_PORTS */
			if (port_count < MAX_PORTS)
				ports[port_count++] = current_port;
		}
	}

	flush_vectors(ports, &port_count);

	fclose(fp);
	unlink(outfile);
	return 0;
}
