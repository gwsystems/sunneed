#include "sunneed_core.h"

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, ":h")) != -1) {
        switch (opt) {
            case 'h':  
                printf(HELP_TEXT, argv[0]);
                exit(0);
        }
    }

    return 0;
}
