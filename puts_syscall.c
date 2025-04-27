// Compilar com: gcc -nostdlib -no-pie -o puts_syscall puts_syscall.c

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

// Função para obter o tamanho da string (sem usar strlen da libc)
uint64_t strlen(const char *s) {
    uint64_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

// Função puts(str) usando apenas syscall write
void puts_syscall(const char *s) {
    uint64_t len = strlen(s);

    asm volatile (
        "movq $1, %%rax\n\t"       // syscall: sys_write
        "movq $1, %%rdi\n\t"       // fd: 1 (stdout)
        "movq %0, %%rsi\n\t"       // buf: ponteiro para a string
        "movq %1, %%rdx\n\t"       // count: tamanho da string
        "syscall\n\t"
        :
        : "r"(s), "r"(len)
        : "rax", "rdi", "rsi", "rdx"
    );
}

// Função _start(), sem main()
void _start() {
    puts_syscall("Hello, syscall world!\n");

    // syscall exit(0)
    asm volatile (
        "movq $60, %%rax\n\t"      // syscall: sys_exit
        "xorq %%rdi, %%rdi\n\t"    // status: 0
        "syscall\n\t"
        :
        :
        : "rax", "rdi"
    );
}


