#include <stdio.h>
#include <assert.h>

#define BUFFER_SIZE 1024

typedef unsigned char byte;

void decode(const byte buffer[], size_t n);
unsigned decode_mov(const byte buffer[], unsigned i, size_t n);
char* lookup_register(byte w, byte reg);

int main(int argc, char *argv[]) {
    // assert one command line arg passed
    assert(argc == 2);

    byte buffer[BUFFER_SIZE];   // buffer for holding bytes
    FILE *fp;                   // file handle

    // open file in read mode
    fp = fopen(argv[1], "r");
    assert(fp != NULL);

    // read raw bytes into buffer
    size_t bytes_read = fread(buffer, sizeof(byte), BUFFER_SIZE, fp);
    assert(bytes_read != 0);

    // decode bytes as instructions
    decode(buffer, bytes_read);

    // close file
    fclose(fp);

    return 0;
}

// decodes op code and calls relevant decoder
void decode(const byte buffer[], size_t n) {
    unsigned i = 0;
    byte op_code, d, w;

    while (i < n) {
        op_code = buffer[i] >> 2;
        switch (op_code) {
            case 0b100010:  // MOV
                i = decode_mov(buffer, i, n);
                break;
            default:
                break;
        }
    }
}

// decodes the MOV instruction
unsigned decode_mov(const byte buffer[], unsigned i, size_t n) {
    byte d, w, mod, reg, rm;
    d = (buffer[i] >> 1) & 1;       // whether reg field is destination (1) or source (0)
    w = buffer[i] & 1;              // whether this is a word (1) or byte (0) operation
    i++;
    mod = buffer[i] >> 6;           // register mode
    reg = (buffer[i] >> 3) & 0b111; // register field encoding
    rm = buffer[i] & 0b111;         // register/memory field encoding

    if (d == 1) {
        printf("mov %s, %s\n", lookup_register(w, reg), lookup_register(w, rm));
    } else {
        printf("mov %s, %s\n", lookup_register(w, rm), lookup_register(w, reg));
    }

    return ++i;
}

// see chapter 4, page 20 of 8086 manual for these tables.
char* lookup_register(byte w, byte reg) {
    if (w == 0) {
        switch (reg) {
            case 0b000:
                return "al";
            case 0b001:
                return "cl";
            case 0b010:
                return "dl";
            case 0b011:
                return "bl";
            case 0b100:
                return "ah";
            case 0b101:
                return "ch";
            case 0b110:
                return "dl";
            case 0b111:
                return "bl";
            default:
                assert(1 == 0); // should not reach here
        }
    } else {
        switch (reg) {
            case 0b000:
                return "ax";
            case 0b001:
                return "cx";
            case 0b010:
                return "dx";
            case 0b011:
                return "bx";
            case 0b100:
                return "sp";
            case 0b101:
                return "bp";
            case 0b110:
                return "si";
            case 0b111:
                return "di";
            default:
                assert(1 == 0); // should not reach here
        }
    }
}