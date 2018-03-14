/*******************************************************************************
 *   Ledger Nimiq App
 *   (c) 2017 Ledger
 *
 *  adapted from https://github.com/mjg59/tpmtotp/blob/master/base32.h
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "nimiq_utils.h"
#include "blake2b.h"
#include "base32.h"

void iban_check(char in[36], char *check) {
    unsigned int counter = 0;
    unsigned int offset = 0;
    unsigned int modulo = 0;

    int partial_uint = 0;

    char total_number[71] = { 0 };
    char partial_number[10] = { 0 };

    // Convert the address to a number-only string
    for (unsigned int i = 0; i < 36; i++) {
        if (70 >= counter) {
            // XXX buffer overflow, signal error
        }
        if (in[i] >= 48 && in[i] <= 57) {
            total_number[counter++] = in[i];
        } else if (in[i] >= 65 && in[i] <= 90) {
            snprintf(&total_number[counter++], 3, "%d", in[i] - 55);
            // Letters convert to a two digit number, increase the counter one more time
            counter++;
        } else if (in[i] >= 97 && in[i] <= 122) {
            snprintf(&total_number[counter++], 3, "%d", in[i] - 87);
            // Letters convert to a two digit number, increase the counter one more time
            counter++;
        } else {
            // XXX unknown ascii code, signal error
        }
    }

    // Compute modulo-97 on the resulting number (do it in 32-bit pieces)
    counter = 0;
    for (unsigned int i = 0; i < 9; i++) {
        strncpy(&partial_number[offset], &total_number[counter], 9 - offset);
        // strncpy(check, partial_number, 10);
        counter += 9 - offset;
        for (unsigned int j = 0; j < 9; j++) {
            if (partial_number[j] != '\0') {
                partial_uint = 10 * partial_uint + (partial_number[j] - '0');
            } else {
                break;
            }
        }

        modulo = partial_uint % 97;
        snprintf(partial_number, 3, "%02d", modulo);
        partial_uint = 0;
        offset = 2;
    }

    snprintf(check, 3, "%02d", 98 - modulo);
}

void print_public_key(uint8_t *in, char *out) {
    unsigned int counter = 4;

    uint8_t after_blake[32] = { 0 };
    char after_base32[36] = { 0 };

    blake2b(after_blake, 32, NULL, 0, in, 32);

    base32_encode(after_blake, 20, after_base32, 32);

    after_base32[32] = 'N';
    after_base32[33] = 'Q';
    after_base32[34] = '0';
    after_base32[35] = '0';
    iban_check(after_base32, &out[2]);

    out[0] = 'N';
    out[1] = 'Q';

    // Insert spaces for readability
    for (unsigned int i = 0; i < 8; i++) {
        out[counter++] = ' ';
        memcpy(&out[counter], &after_base32[i*4], 4);
        counter += 4;
    }

    out[44] = '\0';
}
