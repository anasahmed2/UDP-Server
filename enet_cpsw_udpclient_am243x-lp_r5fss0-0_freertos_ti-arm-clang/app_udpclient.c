/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "app_udpclient.h" // Keep original header for build compatibility

#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/* FreeRTOS Headers for Threading and Mutex */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/CacheP.h>
#include "enet_apputils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define SERVER_UDP_PORT     (8016)
#define MAX_LOGS            (30)  // Store the last 30 logs
#define TX_BUFFER_SIZE      (1024)
#define RX_BUFFER_SIZE      (128)

/* Requirement: Data structure with at least 3 items */
typedef struct {
    uint32_t timestamp;
    uint8_t  type;          // 0 = INFO, 1 = WARN, 2 = ERROR
    float    temperature;
    float    voltage;
} LogItem_t;

/* ========================================================================== */
/*                          THOMAS'S PROTOCOL MACROS                          */
/* ========================================================================== */
#define PACKET_MAGIC    0x1234
#define PKT_ID          0
#define PKT_CMD         2
#define PKT_SEQ         3
#define PKT_TOT         5
#define PKT_IDX         7
#define PKT_LEN         9
#define PKT_DATA        14

#define ENTRY_STATUS    0
#define ENTRY_TS        1
#define ENTRY_TYPE      9
#define ENTRY_MSG       10

#define CMD_GET_ALL_LOGS 0x00 // Matched to his CMD_GET_ALL_LOGS enum 
#define MAX_MSG_LEN      256

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static LogItem_t gLogBuffer[MAX_LOGS];
static uint32_t gLogIndex = 0;
static uint32_t gTotalLogs = 0;
static SemaphoreHandle_t gLogMutex = NULL;

/* ========================================================================== */
/*                          Protocol Helpers                                  */
/* ========================================================================== */

/* Helper to write 16-bit int to buffer (Big/Little Endian safe) */
static void write_u16(uint8_t *b, uint16_t v) {
    b[0] = (v >> 8) & 0xFF; 
    b[1] = v & 0xFF;
}

/* Helper to write 64-bit int to buffer */
static void write_u64(uint8_t *b, uint64_t v) {
    for (int i = 7; i >= 0; i--) { 
        b[i] = v & 0xFF; 
        v >>= 8; 
    }
}

/* Helper to calculate Thomas's specific checksum format */
static uint16_t calc_checksum(const uint8_t *buf, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len - 2; i++) {
        sum += buf[i];
    }
    return sum;
}

/* Helper to read 16-bit int from Thomas's packet */
static uint16_t read_u16(const uint8_t *b) {
    return (uint16_t)((b[0] << 8) | b[1]);
}

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* THREAD 1: Data Generator Task (Runs every 100ms) */
static void AppSocket_dataGeneratorTask(void *pArg)
{
    // Generate UNIX epoch timestamp in MS to match his ISO date format
    // Starting at an arbitrary recent timestamp to look normal
    uint64_t currentTimeMs = 1774890000000; 

    EnetAppUtils_print("Data Generator Thread Started...\r\n");

    while (1)
    {
        /* Lock Mutex to prevent UDP server from reading while we write */
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            // Generate simulated telemetry data
            gLogBuffer[gLogIndex].timestamp = (uint32_t)(currentTimeMs & 0xFFFFFFFF); 
            gLogBuffer[gLogIndex].type = rand() % 3; 
            gLogBuffer[gLogIndex].temperature = 20.0f + ((float)(rand() % 100) / 10.0f);
            gLogBuffer[gLogIndex].voltage = 5.0f - ((float)(rand() % 10) / 10.0f);

            gLogIndex = (gLogIndex + 1) % MAX_LOGS; // Circular buffer wrap
            if (gTotalLogs < MAX_LOGS) gTotalLogs++;

            currentTimeMs += 100;

            xSemaphoreGive(gLogMutex); /* Unlock Mutex */
        }

        /* Sleep exactly 100ms per requirement */
        ClockP_sleep(0); // Yield
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* THREAD 2: UDP Server Task (Listens & Responds to Thomas in Binary) */
static void AppSocket_udpServerTask(void *pArg)
{
    int32_t sock = -1, ret = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint8_t tx_buffer[TX_BUFFER_SIZE];

    EnetAppUtils_print("UDP Server Thread Started on Port %d...\r\n", SERVER_UDP_PORT);

    /* Create the socket */
    sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        vTaskDelete(NULL);
    }

    /* Bind to 8016 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PP_HTONS(SERVER_UDP_PORT);
    server_addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    while (1)
    {
        /* Block and wait for Thomas to press a key and send a request */
        int len = lwip_recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0,
                                (struct sockaddr *)&client_addr, &client_len);

        if (len >= 14) // Thomas's header is at least 14 bytes
        {
            // 1. Verify Magic Number to ensure it's Thomas's software
            if (read_u16(&rx_buffer[PKT_ID]) == PACKET_MAGIC) 
            {
                // 2. Extract his specific Sequence ID!
                uint16_t req_seq = read_u16(&rx_buffer[PKT_SEQ]);
                uint8_t  req_cmd = rx_buffer[PKT_CMD];

                // PRINT: Who sent the request?
                char client_ip[16];
                ip4addr_ntoa_r((const ip4_addr_t *)&client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
                EnetAppUtils_print("\r\n--- Received Request from %s:%d ---\r\n", client_ip, ntohs(client_addr.sin_port));
                EnetAppUtils_print("Client Sequence ID: %d, Command: %d\r\n", req_seq, req_cmd);

                /* Lock Mutex to safely read logs */
                if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
                {
                    int logs_to_send = (gTotalLogs < 10) ? gTotalLogs : 10;
                    int start_idx = (gTotalLogs < 10) ? 0 : gTotalLogs - 10;
                    
                    EnetAppUtils_print("Packaging and sending %d logs...\r\n", logs_to_send);

                    for (int i = 0; i < logs_to_send; i++)
                    {
                        int actual_idx = (start_idx + i) % MAX_LOGS;
                        LogItem_t *log = &gLogBuffer[actual_idx];

                        memset(tx_buffer, 0, sizeof(tx_buffer));

                        // 3. Format Payload
                        uint8_t payload[256];
                        memset(payload, 0, sizeof(payload));
                        
                        payload[ENTRY_STATUS] = 0; 
                        uint64_t fake_timestamp_ms = 1774890000000 + log->timestamp;
                        write_u64(&payload[ENTRY_TS], fake_timestamp_ms); 
                        payload[ENTRY_TYPE] = log->type; 
                        
                        char msg[MAX_MSG_LEN];
                        snprintf(msg, sizeof(msg), "Temp: %.1fC, Volts: %.1fV", log->temperature, log->voltage);
                        int msg_len = strlen(msg);
                        memcpy(&payload[ENTRY_MSG], msg, msg_len);
                        
                        uint16_t payload_len = ENTRY_MSG + msg_len;

                        // PRINT: Show exactly what text is going into this binary packet
                        EnetAppUtils_print("  -> Pkt %d: Type=%d, %s\r\n", i, log->type, msg);

                        // 4. Build Header - ECHOING HIS SEQUENCE ID
                        write_u16(&tx_buffer[PKT_ID], PACKET_MAGIC);
                        tx_buffer[PKT_CMD] = req_cmd;
                        write_u16(&tx_buffer[PKT_SEQ], req_seq); 
                        write_u16(&tx_buffer[PKT_TOT], logs_to_send);
                        write_u16(&tx_buffer[PKT_IDX], i); 
                        write_u16(&tx_buffer[PKT_LEN], payload_len);
                        
                        memcpy(&tx_buffer[PKT_DATA], payload, payload_len);

                        // 5. Checksum and Send DIRECTLY back to WSL
                        int total_pkt_len = PKT_DATA + payload_len + 2;
                        uint16_t checksum = calc_checksum(tx_buffer, total_pkt_len);
                        write_u16(&tx_buffer[PKT_DATA + payload_len], checksum);

                        ret = lwip_sendto(sock, tx_buffer, total_pkt_len, 0,
                                          (struct sockaddr *)&client_addr, client_len); 
                        
                        if (ret < 0) {
                            EnetAppUtils_print("ERR: Failed to send packet %d\r\n", i);
                        }
                    }
                    EnetAppUtils_print("Done sending response.\r\n");

                    xSemaphoreGive(gLogMutex);
                }
            }
            else {
                EnetAppUtils_print("Ignored packet: Invalid Magic Number\r\n");
            }
        }
    }
}


/* Replaces AppSocket_startClient in the original file */
void AppSocket_startServer(void)
{
    /* Create Mutex */
    gLogMutex = xSemaphoreCreateMutex();
    EnetAppUtils_assert(gLogMutex != NULL);

    /* Spawn Thread 1: Data Generator */
    sys_thread_new("LogGenTask", AppSocket_dataGeneratorTask, NULL, 
                   DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    /* Spawn Thread 2: UDP Server */
    sys_thread_new("UdpSrvTask", AppSocket_udpServerTask, NULL, 
                   DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
