#include <drivers/serial.h>
#include <cpu/ports.h>
#include <minilib.h>
#include <ktype.h>


void serial_enable(int device) {
    port_byte_out(device + 1, 0x00);
    port_byte_out(device + 3, 0x80); // enable divisor mode
    port_byte_out(device + 0, 0x03); // div Low:  03 Set the port to 38400 bps
    port_byte_out(device + 1, 0x00); // div High: 00
    port_byte_out(device + 3, 0x03);
    port_byte_out(device + 2, 0xC7);
    port_byte_out(device + 4, 0x0B);
}

int serial_init(void) {
    serial_enable(SERIAL_PORT_A);
    serial_enable(SERIAL_PORT_B);
    return 0;
}

int serial_transmit_empty(int device) {
    return port_byte_in(device + 5) & 0x20;
}

void serial_send(int device, char out) {
    while (serial_transmit_empty(device) == 0);
    port_byte_out(device, out);
}

void serial_print(int device, char *out) {
    for (uint32_t i = 0; i < (uint32_t) str_len(out); ++i) {
        serial_send(device, out[i]);
    }
}
