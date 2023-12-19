#include <stdio.h>
#include <assert.h>

#define BUFFER_SIZE 1024

typedef unsigned char byte;

void decode(const byte buffer[], size_t n);
unsigned decode_move_im_to_reg(const byte buffer[], unsigned i, size_t n);
unsigned decode_mov_rm_to_from_reg(const byte buffer[], unsigned i, size_t n);
char* lookup_register(byte w, byte reg);
char* lookup_effective_address(byte rm);

int main(int argc, char *argv[]) {
    // assert that exactly one command line arg is passed: the file path
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
    byte op_code;

    while (i < n) {
        op_code = buffer[i];

        // check 8-bit op codes

        // check 7-bit op codes
        op_code >>= 1;

        // check 6-bit op codes
        op_code >>= 1;
        if (op_code == 0b100010) {
            i = decode_mov_rm_to_from_reg(buffer, i, n);
            continue;
        }

        // check 4-bit op codes
        op_code >>= 2;
        if (op_code == 0b1011) {
            i = decode_move_im_to_reg(buffer, i, n);
            continue;
        }

        i++;
    }
}

// MOV immediate to register
unsigned decode_move_im_to_reg(const byte buffer[], unsigned i, size_t n) {
    byte w, reg;
    w = (buffer[i] >> 3) & 1;       // whether the immediate is 8-bits (0) or 16-bits (1)
    reg = buffer[i] & 0b111;        // destination register field encoding
    char* reg_name = lookup_register(w, reg);   // decoded register
    i++;
    if (w) {
        i++;
        unsigned short data = buffer[i-1] | (buffer[i] << 8);   // 16 bits of data goes in register
        printf("mov %s, %d\n", reg_name, data);
    } else {
        printf("mov %s, %d\n", reg_name, buffer[i]);          // 8 bits of data goes in register
    }
    return ++i;
}

// MOV register/memory to or from register
unsigned decode_mov_rm_to_from_reg(const byte buffer[], unsigned i, size_t n) {
    byte d, w, mod, reg, rm;
    d = (buffer[i] >> 1) & 1;       // whether reg field is destination (1) or source (0)
    w = buffer[i] & 1;              // whether this is a word (1) or byte (0) operation
    i++;
    mod = buffer[i] >> 6;           // register mode
    reg = (buffer[i] >> 3) & 0b111; // register field encoding
    rm = buffer[i] & 0b111;         // register/memory field encoding

    char *dest, *source;
    if (mod == 0b11) {          // register mode
        dest = (d == 1) ? lookup_register(w, reg) : lookup_register(w, rm);
        source = (d == 1) ? lookup_register(w, rm) : lookup_register(w, reg);
        printf("mov %s, %s\n", dest, source);
    } else if (mod == 0b00) {   // memory mode: no displacement (unless rm == 0b110)
        if (d == 1) {
            printf("mov %s, [%s]\n", lookup_register(w, reg), lookup_effective_address(rm));
        } else {
            printf("mov [%s], %s\n", lookup_effective_address(rm), lookup_register(w, reg));
        }
    } else if (mod == 0b01) {   // memory mode: 8-bit displacement
        i++;
        if (d == 1) {
            printf("mov %s, [%s + %d]\n", lookup_register(w, reg), lookup_effective_address(rm), buffer[i]);
        } else {
            printf("mov [%s + %d], %s\n", lookup_effective_address(rm), buffer[i], lookup_register(w, reg));
        }
    } else if (mod == 0b10) {   // memory mode: 16-bit displacement
        i += 2;
        unsigned short address = buffer[i-1] | (buffer[i] << 8);
        if (d == 1) {
            printf("mov %s, [%s + %d]\n", lookup_register(w, reg), lookup_effective_address(rm), address);
        } else {
            printf("mov [%s + %d], %s\n", lookup_effective_address(rm), address, lookup_register(w, reg));
        }
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

// see chapter 4, page 20 of 8086 manual for table
char* lookup_effective_address(byte rm) {
    switch (rm) {
        case 0b000:
            return "bx + si";
        case 0b001:
            return "bx + di";
        case 0b010:
            return "bp + si";
        case 0b011:
            return "bp + di";
        case 0b100:
            return "si";
        case 0b101:
            return "di";
        case 0b110:
            return "bp";
        case 0b111:
            return "bx";
        default:
            assert(0 == 1); // should not reach here
    }
}
