/* Fills four words from one value: the value itself and its xor with three constants (behaviour
 * inferred; the original name is unknown). */
void fn_8040CB2C(unsigned int *values, unsigned int value) {
    values[0] = value;
    values[1] = value ^ 0x3CC19185;
    values[2] = value ^ 0x01C9C2D0;
    values[3] = value ^ 0x2E43C13E;
}
