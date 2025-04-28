
/*
	cWrapper.c

	Set of functions wrappers to common C library functions.

	Miguel Leitao, 2025
*/

#include <stdarg.h>

#define NULL ((void *)0)


typedef unsigned long size_t;
typedef long ssize_t;
typedef unsigned int uint32_t;

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == 0 || s2[i] == 0)
            return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    return 0;
}

int atoi(const char *s) {
    int num = 0, sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        num = num * 10 + (*s - '0');
        s++;
    }
    return sign * num;
}

// syscall write
static void sys_write(int fd, const char *buf, size_t len) {
    asm volatile (
        "movq $1, %%rax\n\t"  // sys_write
        "movq %0, %%rdi\n\t"
        "movq %1, %%rsi\n\t"
        "movq %2, %%rdx\n\t"
        "syscall\n\t"
        :
        : "r"((long)fd), "r"(buf), "r"(len)
        : "rax", "rdi", "rsi", "rdx"
    );
}

// Função puts mínima
int puts(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }

    // Escreve a string seguida de nova linha
    sys_write(1, str, len);         // escreve a string
    sys_write(1, "\n", 1);          // escreve a nova linha

    return len + 1; // Retorna o número de caracteres (incluindo '\n')
}

int putchar(int c) {
	//char buf[2];
	//buf[0] = c;
	//buf[1] = 0;
	sys_write(1, (const char *)&c, 1);
	return c;
}

// Converte inteiro para string (base 10 ou 16)
static void itoa(long num, int base, char *out) {
    const char *digits = "0123456789abcdef";
    char buf[32];
    int i = 0;

    if (num == 0) {
        out[0] = '0';
        out[1] = 0;
        return;
    }

    int negative = 0;
    if (base == 10 && num < 0) {
        negative = 1;
        num = -num;
    }

    while (num) {
        buf[i++] = digits[num % base];
        num /= base;
    }

    if (negative)
        buf[i++] = '-';

    for (int j = 0; j < i; j++) {
        out[j] = buf[i - j - 1];
    }
    out[i] = 0;
}

void ftoa(float num, char *out) {
    int int_part = (int)num;         		// Parte inteira
    float frac_part = num - int_part; 		// Parte fracionária
    if (frac_part < 0) frac_part = -frac_part; // Corrige caso negativo

    // Converte parte inteira
    int i = 0;
    if (num < 0 && int_part == 0)
        out[i++] = '-';
    if (int_part == 0) {
        out[i++] = '0';
    } else {
        // Preenche número invertido
        int int_temp = int_part;
        char temp[20];
        int j = 0;
        if (int_temp < 0) int_temp = -int_temp;
        while (int_temp) {
            temp[j++] = '0' + (int_temp % 10);
            int_temp /= 10;
        }
        // Copia na ordem certa
        if (int_part < 0) out[i++] = '-';
        while (j--) {
            out[i++] = temp[j];
        }
    }
    out[i++] = '.'; // Ponto decimal
    // Converte parte fracionária (usa 6 dígitos)
    for (int k = 0; k < 6; k++) {
        frac_part *= 10.0f;
        int digit = (int)frac_part;
        out[i++] = '0' + digit;
        frac_part -= digit;
    }
    out[i] = '\0'; // Finaliza a string
}

// printf mínimo
int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    #ifdef _DEBUG_
		sys_write(1, "printf(", 7);
		sys_write(1, fmt, strlen(fmt) );
		sys_write(1, ")\n", 1);
	#endif	
    char buf[532];
    for (size_t i = 0; fmt[i]; i++) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 's') {
                const char *str = va_arg(ap, const char *);
                sys_write(1, str, strlen(str));
            } else if (fmt[i] == 'd' || fmt[i] == 'f') {
                int val = va_arg(ap, int);
                itoa(val, 10, buf);
                sys_write(1, buf, strlen(buf));
            #ifdef _PRINTF_FLOAT_    
            } else if (fmt[i] == 'g') {
                sys_write(1, "float\n", 6);
                //double val = va_arg(ap, double);
                double val=9.3;
                int vi = (int)val;
				itoa(vi, 10, buf);
                sys_write(1, buf, strlen(buf));
                ftoa((float)val, buf);
                sys_write(1, buf, strlen(buf));
            #endif    
            } else if (fmt[i] == 'x') {
                int val = va_arg(ap, int);
                itoa(val, 16, buf);
                sys_write(1, buf, strlen(buf));
            }
        } else {
            sys_write(1, &fmt[i], 1);
        }
    }
    va_end(ap);
    return 0;
}

// syscall helpers
static long syscall3(long n, long a1, long a2, long a3) {
    long ret;
    asm volatile (
        "movq %1, %%rax\n\t"
        "movq %2, %%rdi\n\t"
        "movq %3, %%rsi\n\t"
        "movq %4, %%rdx\n\t"
        "syscall\n\t"
        : "=a" (ret)
        : "r"(n), "r"(a1), "r"(a2), "r"(a3)
        : "rdi", "rsi", "rdx"
    );
    return ret;
}

static long syscall1(long n, long a1) {
    long ret;
    asm volatile (
        "movq %1, %%rax\n\t"
        "movq %2, %%rdi\n\t"
        "syscall\n\t"
        : "=a" (ret)
        : "r"(n), "r"(a1)
        : "rdi"
    );
    return ret;
}

// fork
static int fork() {
    return syscall3(57, 0, 0, 0); // sys_fork
}

// execve
static int execve(const char *filename, char *const argv[], char *const envp[]) {
    return syscall3(59, (long)filename, (long)argv, (long)envp);
}

// waitpid
static int waitpid(int pid, int *status, int options) {
    return syscall3(61, pid, (long)status, options); // sys_wait4
}

// system() usando fork+execve+waitpid
int system(const char *cmd) {
    if (!cmd) return 1;

    int pid = fork();
    if (pid == 0) {
        // filho
        char *argv[] = {"/bin/sh", "-c", (char *)cmd, NULL};
        char *envp[] = {NULL};
        execve("/bin/sh", argv, envp);
        syscall1(60, 1); // exit(1) se execve falhar
    } else {
        // pai
        int status=0;
        waitpid(pid, &status, 0);
        return status;
    }
    return 0;
}

static ssize_t syscall_read(int fd, void *buf, size_t count) {
    ssize_t ret;
    asm volatile (
        "movq $0, %%rax\n\t"        // syscall number para sys_read (0)
        "movq %1, %%rdi\n\t"        // fd (primeiro argumento)
        "movq %2, %%rsi\n\t"        // buffer (segundo argumento)
        "movq %3, %%rdx\n\t"        // count (terceiro argumento)
        "syscall\n\t"
        "movq %%rax, %0\n\t"        // return value
        : "=r" (ret)                 // armazena o valor de retorno
        : "r"((long)fd), "r"(buf), "r"(count)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );
    return ret;
}


// scanf mínimo: só suporta "%s" e "%d"
int scanf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char buf[128];
    int len = syscall_read(0, buf, sizeof(buf) - 1);
    if (len <= 0) return -1;
    buf[len] = 0;

    int ret = 0;
    int i = 0, j = 0;
    while (fmt[i]) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 's') {
                char *out = va_arg(ap, char *);
                j = 0;
                while (buf[j] && buf[j] != '\n' && buf[j] != ' ') {
                    out[j] = buf[j];
                    j++;
                }
                out[j] = 0;
                ret++;
            } else if (fmt[i] == 'd') {
                int *out = va_arg(ap, int *);
                *out = atoi(buf);
                ret++;
            }
        }
        i++;
    }
    va_end(ap);
    return ret;
}

#ifdef TEST_APP

void _start() {	
	puts("Testing puts:\n  Hello World.");
	puts("Testing putchar:");
	    putchar(' ');
	    putchar(' '); 
	    putchar('O');
	    putchar('K');
	    putchar('\n');
	puts("Testing printf:");
    printf("  int: %d+%d=%d\n", 65,   27,   65+27);
    #ifdef _PRINTF_FLOAT_
    printf("  float: %f+%f=%f\n", 4.5f, 5.3f, 9.8f);
    #endif
    printf("  str: '%s'\n", "all done");
    printf("Testing atoi:\n  '2735'=%d\n", atoi("2735"));
    
    // Exit
        asm volatile (
        "movq $60, %%rax\n\t" // sys_exit
        "xorq %%rdi, %%rdi\n\t"
        "syscall\n\t"
        :
        :
        : "rax", "rdi"
    );
}

#endif
