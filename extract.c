#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_INPUT 1000

int extractIPv4(const char *input, uint32_t *address, int *port, int *hasPort) {
    int i = 0;

    while (input[i] != '\0') {

        // Skip everything that cannot be part of a candidate token
        if (!isdigit((unsigned char)input[i]) &&
            input[i] != '.' &&
            input[i] != ':') {
            i++;
            continue;
        }

        // Find the end of the candidate token
        int start = i;

        while (input[i] != '\0' &&
               (isdigit((unsigned char)input[i]) ||
                input[i] == '.' ||
                input[i] == ':')) {
            i++;
        }

        int end = i;

        // Check the entire candidate token
        int pos = start;
        int octets[4];
        int valid = 1;

        for (int octet = 0; octet < 4; octet++) {
            int digits = 0;
            int value = 0;

            // Must have at least one digit
            if (pos >= end || !isdigit((unsigned char)input[pos])) {
                valid = 0;
                break;
            }

            // No leading zero unless the octet is exactly 0
            if (input[pos] == '0') {
                if (pos + 1 < end && isdigit((unsigned char)input[pos + 1])) {
                    valid = 0;
                    break;
                }
            }

            // Read 1-3 digits
            while (pos < end && isdigit((unsigned char)input[pos])) {
                if (digits >= 3) {
                    valid = 0;
                    break;
                }

                value = value * 10 + (input[pos] - '0');
                digits++;
                pos++;
            }

            if (!valid) {
                break;
            }

            // Octet must be 0-255
            if (value > 255) {
                valid = 0;
                break;
            }

            octets[octet] = value;

            // First three octets must be followed by a period
            if (octet < 3) {
                if (pos >= end || input[pos] != '.') {
                    valid = 0;
                    break;
                }

                pos++;
            }
        }

        if (!valid) {
            continue;
        }

        /*
         * At this point the four octets were valid.
         * Check for an optional port.
         */
        int currentPort = 0;
        int currentHasPort = 0;

        if (pos < end) {
            // The only thing allowed after the fourth octet is :port
            if (input[pos] != ':') {
                valid = 0;
            } else {
                pos++;
                currentHasPort = 1;

                int portDigits = 0;

                // Port must contain at least one digit
                if (pos >= end || !isdigit((unsigned char)input[pos])) {
                    valid = 0;
                }

                // No leading zero unless the port is exactly 0
                if (valid && input[pos] == '0') {
                    if (pos + 1 < end && isdigit((unsigned char)input[pos + 1])) {
                        valid = 0;
                    }
                }

                // Read port digits
                while (valid && pos < end && isdigit((unsigned char)input[pos])) {
                    if (portDigits >= 5) {
                        valid = 0;
                        break;
                    }

                    currentPort = currentPort * 10 + (input[pos] - '0');
                    portDigits++;
                    pos++;
                }

                // Port must be 0-65535
                if (valid && currentPort > 65535) {
                    valid = 0;
                }
            }
        }

        // Nothing else can remain in the candidate token
        if (pos != end) {
            valid = 0;
        }

        if (valid) {
            // Build the 32-bit IPv4 value by hand
            *address = ((uint32_t)octets[0] << 24) |
                       ((uint32_t)octets[1] << 16) |
                       ((uint32_t)octets[2] << 8) |
                       (uint32_t)octets[3];

            *port = currentPort;
            *hasPort = currentHasPort;

            return 1;
        }
    }

    return 0;
}

int main(void) {
    char input[MAX_INPUT];

    while (1) {
        printf("Enter a string (or 'END' to quit): ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Check for END without using strcmp()
        if (input[0] == 'E' &&
            input[1] == 'N' &&
            input[2] == 'D' &&
            (input[3] == '\n' || input[3] == '\0')) {

            printf("Program terminated.\n");
            break;
        }

        uint32_t address;
        int port;
        int hasPort;

        if (extractIPv4(input, &address, &port, &hasPort)) {
            unsigned int a = (address >> 24) & 255;
            unsigned int b = (address >> 16) & 255;
            unsigned int c = (address >> 8) & 255;
            unsigned int d = address & 255;

            if (hasPort) {
                printf("Extracted IPv4 address: %u.%u.%u.%u (decimal value: %u, port: %d)\n",
                       a, b, c, d, address, port);
            } else {
                printf("Extracted IPv4 address: %u.%u.%u.%u (decimal value: %u, port: none)\n",
                       a, b, c, d, address);
            }
        } else {
            printf("Invalid input: no valid IPv4 address found\n");
        }
    }

    return 0;
}