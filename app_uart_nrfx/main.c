#include <app_error.h>
#include <app_timer.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>

#include <nrfx_clock.h>
#include <nrfx_uarte.h>

#include <hal/nrf_gpio.h>

// ---

typedef enum {
    RECV_STATE_AWAITING_HEADER_1ST_BYTE,
    RECV_STATE_AWAITING_HEADER_2ND_BYTE,
    RECV_STATE_AWAITING_PACKET_LENGTH,
    RECV_STATE_AWAITING_REST_OF_PACKET
} recv_state_t;

static recv_state_t recv_state = 0;
#define RECV_PACKET_BUF_SIZE 300
static uint8_t recv_packet[RECV_PACKET_BUF_SIZE];
static unsigned int recv_packet_total_size = 0;
static unsigned int recv_packet_bytes_received = 0;
static unsigned int recv_packet_bytes_remaining = 0;

void hexdump(const char * prefix, const uint8_t * const bytes, unsigned int length) {
    NRF_LOG_RAW_INFO(prefix);
    for (int i = 0; i < length; i++) {
        NRF_LOG_RAW_INFO("%02x", bytes[i]);
    }
    NRF_LOG_RAW_INFO("\r\n");
}

void on_packet_received() {
    if (recv_packet[1] != 0x3D && recv_packet[2] != 0x3D) return;
    hexdump("p: ", recv_packet, recv_packet_total_size);
}

void on_recv_data_chunk(uint8_t const * bytes, unsigned int offset, unsigned int length) {
    if (length <= 0) {
        return;
    }

    switch (recv_state) {
        case RECV_STATE_AWAITING_HEADER_1ST_BYTE: {
            for (int i = 0; i < length; i++) {
                if (bytes[offset + i] == 0x5A) {
                    recv_state = RECV_STATE_AWAITING_HEADER_2ND_BYTE;
                    on_recv_data_chunk(bytes, offset + i + 1, length - i - 1);
                    break;
                }
            }
        }
        break;

        case RECV_STATE_AWAITING_HEADER_2ND_BYTE: {
            if (bytes[offset] == 0xA5) {
                recv_state = RECV_STATE_AWAITING_PACKET_LENGTH;
            } else {
                recv_state = RECV_STATE_AWAITING_HEADER_1ST_BYTE;
            }
            on_recv_data_chunk(bytes, offset + 1, length - 1);

        }
        break;

        case RECV_STATE_AWAITING_PACKET_LENGTH: {
            uint8_t recv_packet_length = bytes[offset];

            recv_packet_total_size = 7 + recv_packet_length;
            recv_packet[0] = recv_packet_length;

            recv_packet_bytes_received = 1;
            recv_packet_bytes_remaining = recv_packet_total_size - recv_packet_bytes_received;

            recv_state = RECV_STATE_AWAITING_REST_OF_PACKET;
            on_recv_data_chunk(bytes, offset + 1, length - 1);

        }
        break;

        case RECV_STATE_AWAITING_REST_OF_PACKET: {
            unsigned int bytes_to_add = length <= recv_packet_bytes_remaining
            ? length
            : recv_packet_bytes_remaining;

            for (int i = 0; i < bytes_to_add; i++) {
                recv_packet[recv_packet_bytes_received + i] = bytes[offset + i];
            }

            recv_packet_bytes_received += bytes_to_add;
            recv_packet_bytes_remaining -= bytes_to_add;

            if (recv_packet_bytes_remaining == 0) {
// if (isChecksumOkay(recvPacket)) {
//   var packet = parsePacket(recvPacket);
//   if (IGNORE_NON_IOT_PACKETS === false || packet.target === ADDR_IOT) {
                on_packet_received();
//   }
// } else {
//   print('invalid checksum: ' + toHex(recvPacket.buffer));
// }

                memset(recv_packet, 0, recv_packet_total_size);
                recv_packet_total_size = 0;
                recv_packet_bytes_received = 0;
                recv_packet_bytes_remaining = 0;

                recv_state = RECV_STATE_AWAITING_HEADER_1ST_BYTE;
                on_recv_data_chunk(bytes, offset + bytes_to_add, length - bytes_to_add);
            }

        }
        break;
    }
}

// ---

#define PIN_TX NRF_GPIO_PIN_MAP(0, 6)
#define PIN_RX NRF_GPIO_PIN_MAP(0, 8)

#define UART_RX_SIZE UARTE0_EASYDMA_MAXCNT_SIZE
static nrfx_uarte_t uarte = NRFX_UARTE_INSTANCE(0);
unsigned int uart_rx_idx = 0;
static uint8_t uart_rx_data[2][UART_RX_SIZE];

void uart_do_rx() {
    APP_ERROR_CHECK(nrfx_uarte_rx(&uarte, uart_rx_data[uart_rx_idx], UART_RX_SIZE));
    uart_rx_idx = (uart_rx_idx + 1) % 2;
}

uint8_t packet_read_error_code[] = {0x5A, 0xA5, 0x01, 0x3D, 0x20, 0x01, 0x1B, 0x02, 0x83, 0xFF};
uint8_t packet_read_fw_ver[] = {0x5A, 0xA5, 0x01, 0x3D, 0x20, 0x01, 0x1A, 0x02, 0x84, 0xFF};
uint8_t packet_read_battery_perc[] = {0x5A, 0xA5, 0x01, 0x3D, 0x20, 0x01, 0x22, 0x02, 0x7C, 0xFF};
uint8_t * packets[] = {
    packet_read_error_code,
    packet_read_fw_ver,
    packet_read_battery_perc
};
unsigned int uart_tx_packet = 0;

void uart_do_tx() {
    if (uart_tx_packet >= 3) {
        uart_tx_packet = 0;
        return;
    }
    uint8_t * packet = packets[uart_tx_packet++];
    APP_ERROR_CHECK(nrfx_uarte_tx(&uarte, packet, 10));
}

void uarte_event_handler(nrfx_uarte_event_t const * p_event, void * p_context) {
    switch (p_event->type) {
        case NRFX_UARTE_EVT_TX_DONE:
            uart_do_tx();

            NRF_LOG_INFO("uarte tx: %u", p_event->data.rxtx.bytes);
            hexdump("", p_event->data.rxtx.p_data, p_event->data.rxtx.bytes);

            break;

        case NRFX_UARTE_EVT_RX_DONE:
            uart_do_rx();

            NRF_LOG_INFO("uarte rx: %u", p_event->data.rxtx.bytes);
            hexdump("", p_event->data.rxtx.p_data, p_event->data.rxtx.bytes);

            on_recv_data_chunk(p_event->data.rxtx.p_data, 0, p_event->data.rxtx.bytes);
            break;

        case NRFX_UARTE_EVT_ERROR:
            uart_do_rx();

            NRF_LOG_ERROR("uarte error: %u", p_event->data.error.error_mask);
            hexdump("", p_event->data.rxtx.p_data, p_event->data.rxtx.bytes);

            break;
    }
}

APP_TIMER_DEF(tx_timer);
void tx_timer_handler(void * p_context) {
    NRF_LOG_INFO("tx");
    uart_do_tx();
}

int main(void) {
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    APP_ERROR_CHECK(nrfx_clock_init(NULL));
    nrfx_clock_lfclk_start();

    nrfx_uarte_config_t uarte_config = {
        .pseltxd            = PIN_TX,
        .pselrxd            = PIN_RX,
        .pselcts            = NRF_UARTE_PSEL_DISCONNECTED,
        .pselrts            = NRF_UARTE_PSEL_DISCONNECTED,
        .p_context          = NULL,
        .hwfc               = (nrf_uarte_hwfc_t) NRF_UARTE_HWFC_DISABLED,
        .parity             = (nrf_uarte_parity_t) NRF_UARTE_PARITY_EXCLUDED,
        .baudrate           = (nrf_uarte_baudrate_t) NRF_UARTE_BAUDRATE_115200,
        .interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY,
    };
    APP_ERROR_CHECK(nrfx_uarte_init(&uarte, &uarte_config, uarte_event_handler));

    // set tx to open-drain output
    nrf_gpio_cfg(PIN_TX,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_H0D1,
        NRF_GPIO_PIN_NOSENSE);

    APP_ERROR_CHECK(app_timer_init());
    APP_ERROR_CHECK(app_timer_create(&tx_timer, APP_TIMER_MODE_REPEATED, tx_timer_handler));
    APP_ERROR_CHECK(app_timer_start(tx_timer, APP_TIMER_TICKS(1000), NULL));

    NRF_LOG_INFO("start");

    uart_do_rx();
}
