#include "sunneed_core.h"

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, ":h")) != -1) {
        switch (opt) {
            case 'h':  
                printf(HELP_TEXT, argv[0]);
                exit(0);
            case '?':
                fprintf(stderr, "%s: illegal option -%c\n", APP_NAME, optopt);
                exit(1);
            case ':':
                fprintf(stderr, "%s: expected argument for option -%c\n", APP_NAME, optopt);
                exit(1);
        }
    }

    return 0;
}
