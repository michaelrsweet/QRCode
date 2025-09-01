/**
 * Test program that generates a SVG QR code using the API.
 *
 * Usage:
 *
 *   ./testqrcode [-e {low,medium,quartile,high}] [-v VERSION] TEXT >FILENAME.svg
 *
 * The MIT License (MIT)
 *
 * This library is written and maintained by Richard Moore.
 * Major parts were derived from Project Nayuki's library.
 *
 * Copyright (c) 2025 by Michael R Sweet
 * Copyright (c) 2017 Richard Moore     (https://github.com/ricmoo/QRCode)
 * Copyright (c) 2017 Project Nayuki    (https://www.nayuki.io/page/qr-code-generator-library)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qrcode.h"


// SVG constants...
#define SVG_SCALE    5                  // Nominal size of modules
#define SVG_PADDING  4                  // White padding around QR code


// Main entry
int main(int argc, char *argv[]) {
    int        i;                       // Looping var
    uint8_t    ecc = ECC_LOW;           // Error correction level
    uint8_t    version = VERSION_AUTO;  // Version/size
    const char *text = NULL;            // Text to encode
    QRCode     qrcode;                  // QR code data
    uint8_t    qrcodeBytes[qrcode_getBufferSize(VERSION_MAX)];
                                        // QR code buffer

    // Parse command-line...
    for (i = 1; i < argc; i ++) {
        if (argv[i][0] == '-') {
            for (const char *opt = argv[i] + 1; *opt; opt ++) {
                switch (*opt) {
                    case 'e' : /* -e ECC */
                        i ++;
                        if (i >= argc) {
                            fputs("testqrcode: Missing error correction level after '-e'.\n", stderr);
                            return 1;
                        } else if (!strcmp(argv[i], "low")) {
                            ecc = ECC_LOW;
                        } else if (!strcmp(argv[i], "medium")) {
                            ecc = ECC_MEDIUM;
                        } else if (!strcmp(argv[i], "quartile")) {
                            ecc = ECC_QUARTILE;
                        } else if (!strcmp(argv[i], "high")) {
                            ecc = ECC_HIGH;
                        } else {
                            fprintf(stderr, "testqrcode: Bad error correction level '-e %s'.\n", argv[i]);
                            return 1;
                        }
                        break;

                    case 'v' : /* -v VERSION */
                        i ++;
                        if (i >= argc) {
                            fputs("testqrcode: Missing version number after '-v'.\n", stderr);
                            return 1;
                        } else {
                            long tempval = strtol(argv[i], NULL, 10);
                            if (tempval < VERSION_MIN || tempval > VERSION_MAX) {
                                fprintf(stderr, "testqrcode: Bad version '-v %s'.\n", argv[i]);
                                return 1;
                            }
                            version = (uint8_t)tempval;
                        }
                        break;

                    default :
                        fprintf(stderr, "testqrcode: Unknown option '-%c'.\n", *opt);
                        return 1;
                }
            }
        } else if (text != NULL) {
            fprintf(stderr, "testqrcode: Unknown option '%s'.\n", argv[i]);
            return 1;
        } else {
            text = argv[i];
        }
    }

    // Verify we have something to generate...
    if (text == NULL) {
        fputs("Usage: ./testqrcode [-e ECC] [-v VERSION] TEXT >FILENAME.svg\n", stderr);
        fputs("Options:\n", stderr);
        fputs("-e ECC      Specify error correction (low,medium,quartile,high)\n", stderr);
        fputs("-e VERSION  Specify version/size (1 to 40, default is auto)\n", stderr);
        return 1;
    }

    // Generate QR code...
    if (qrcode_initText(&qrcode, qrcodeBytes, version, ecc, text) < 0) {
        fputs("testqrcode: Unable to generate QR code.\n", stderr);
        return 1;
    }

    // Write SVG to stdout...
    printf("<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n", (qrcode.size + 2 * SVG_PADDING) * SVG_SCALE, (qrcode.size + 2 * SVG_PADDING) * SVG_SCALE);
    printf("  <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"white\" />\n", (qrcode.size + 2 * SVG_PADDING) * SVG_SCALE, (qrcode.size + 2 * SVG_PADDING) * SVG_SCALE);

    for (uint8_t y = 0; y < qrcode.size; y++) {
        uint8_t xstart = 0, xcount = 0;

        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                if (xcount == 0) { xstart = x; }
                xcount ++;
            } else if (xcount > 0) {
                printf("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\" />\n", (xstart + SVG_PADDING) * SVG_SCALE, (y + SVG_PADDING) * SVG_SCALE, xcount * SVG_SCALE, SVG_SCALE);
                xcount = 0;
            }
        }

        if (xcount > 0) {
            printf("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\" />\n", (xstart + SVG_PADDING) * SVG_SCALE, (y + SVG_PADDING) * SVG_SCALE, xcount * SVG_SCALE, SVG_SCALE);
        }
    }

    puts("</svg>");
    return 0;
}
