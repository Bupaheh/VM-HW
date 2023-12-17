/* Lama SM Bytecode interpreter */

# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <stdlib.h>
# include <stdarg.h>
# include "byterun.h"

static void vfailure (char *s, va_list args) {
    fprintf  (stderr, "*** FAILURE: ");
    vfprintf (stderr, s, args); // vprintf (char *, va_list) <-> printf (char *, ...)
    exit     (255);
}

void failure (const char *s, ...) {
    va_list args;

    va_start (args, s);
    vfailure ((char *) s, args);
}

/* Gets a string from a string table by an index */
char* get_string (bytefile *f, int pos) {
    return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char* get_public_name (bytefile *f, int i) {
    return get_string (f, f->public_ptr[i*2]);
}

/* Gets an offset for a publie symbol */
int get_public_offset (bytefile *f, int i) {
    return f->public_ptr[i*2+1];
}

/* Reads a binary bytecode file by name and unpacks it */
bytefile* read_file (char *fname) {
    FILE *f = fopen (fname, "rb");
    long size;
    bytefile *file;

    if (f == 0) {
        failure ("%s\n", strerror (errno));
    }

    if (fseek (f, 0, SEEK_END) == -1) {
        failure ("%s\n", strerror (errno));
    }

    file = (bytefile*) malloc (sizeof(int)*4 + (size = ftell (f)));

    if (file == 0) {
        failure ("*** FAILURE: unable to allocate memory.\n");
    }

    rewind (f);

    if (size != fread (&file->stringtab_size, 1, size, f)) {
        failure ("%s\n", strerror (errno));
    }

    fclose (f);

    file->string_ptr  = &file->buffer [file->public_symbols_number * 2 * sizeof(int)];
    file->public_ptr  = (int*) file->buffer;
    file->code_ptr    = &file->string_ptr [file->stringtab_size];
    file->global_ptr  = (int*) malloc (file->global_area_size * sizeof (int));

    return file;
}

void log_null(FILE *f, const char *format, ...) {
    if (f == NULL) return;

    va_list args;
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
}

char *disassemble_instruction(FILE *f, bytefile *bf, char *ip) {

# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   failure ("ERROR: invalid opcode %d-%d\n", h, l)

    char *ops [] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
    char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
    char *lds [] = {"LD", "LDA", "ST"};

    char x = BYTE,
            h = (x & 0xF0) >> 4,
            l = x & 0x0F;

    switch (h) {
        case 15:
            log_null (f, "STOP");
            ip = NULL;
            break;

            /* BINOP */
        case 0:
            log_null (f, "BINOP\t%s", ops[l-1]);
            break;

        case 1:
            switch (l) {
                case  0:
                    log_null (f, "CONST\t%d", INT);
                    break;

                case  1:
                    log_null (f, "STRING\t%s", STRING);
                    break;

                case  2:
                    log_null (f, "SEXP\t%s ", STRING);
                    log_null (f, "%d", INT);
                    break;

                case  3:
                    log_null (f, "STI");
                    break;

                case  4:
                    log_null (f, "STA");
                    break;

                case  5:
                    log_null (f, "JMP\t0x%.8x", INT);
                    break;

                case  6:
                    log_null (f, "END");
                    break;

                case  7:
                    log_null (f, "RET");
                    break;

                case  8:
                    log_null (f, "DROP");
                    break;

                case  9:
                    log_null (f, "DUP");
                    break;

                case 10:
                    log_null (f, "SWAP");
                    break;

                case 11:
                    log_null (f, "ELEM");
                    break;

                default:
                    FAIL;
            }
            break;

        case 2:
        case 3:
        case 4:
            log_null (f, "%s\t", lds[h-2]);
            switch (l) {
                case 0: log_null (f, "G(%d)", INT); break;
                case 1: log_null (f, "L(%d)", INT); break;
                case 2: log_null (f, "A(%d)", INT); break;
                case 3: log_null (f, "C(%d)", INT); break;
                default: FAIL;
            }
            break;

        case 5:
            switch (l) {
                case  0:
                    log_null (f, "CJMPz\t0x%.8x", INT);
                    break;

                case  1:
                    log_null (f, "CJMPnz\t0x%.8x", INT);
                    break;

                case  2:
                    log_null (f, "BEGIN\t%d ", INT);
                    log_null (f, "%d", INT);
                    break;

                case  3:
                    log_null (f, "CBEGIN\t%d ", INT);
                    log_null (f, "%d", INT);
                    break;

                case  4:
                    log_null (f, "CLOSURE\t0x%.8x", INT);
                    {int n = INT;
                        for (int i = 0; i<n; i++) {
                            switch (BYTE) {
                                case 0: log_null (f, "G(%d)", INT); break;
                                case 1: log_null (f, "L(%d)", INT); break;
                                case 2: log_null (f, "A(%d)", INT); break;
                                case 3: log_null (f, "C(%d)", INT); break;
                                default: FAIL;
                            }
                        }
                    };
                    break;

                case  5:
                    log_null (f, "CALLC\t%d", INT);
                    break;

                case  6:
                    log_null (f, "CALL\t0x%.8x ", INT);
                    log_null (f, "%d", INT);
                    break;

                case  7:
                    log_null (f, "TAG\t%s ", STRING);
                    log_null (f, "%d", INT);
                    break;

                case  8:
                    log_null (f, "ARRAY\t%d", INT);
                    break;

                case  9:
                    log_null (f, "FAIL\t%d", INT);
                    log_null (f, "%d", INT);
                    break;

                case 10:
                    log_null (f, "LINE\t%d", INT);
                    break;

                default:
                    FAIL;
            }
            break;

        case 6:
            log_null (f, "PATT\t%s", pats[l]);
            break;

        case 7: {
            switch (l) {
                case 0:
                    log_null (f, "CALL\tLread");
                    break;

                case 1:
                    log_null (f, "CALL\tLwrite");
                    break;

                case 2:
                    log_null (f, "CALL\tLlength");
                    break;

                case 3:
                    log_null (f, "CALL\tLstring");
                    break;

                case 4:
                    log_null (f, "CALL\tBarray\t%d", INT);
                    break;

                default:
                    FAIL;
            }
        }
            break;

        default:
            FAIL;
    }

    log_null (f, "\n");
    return ip;
}