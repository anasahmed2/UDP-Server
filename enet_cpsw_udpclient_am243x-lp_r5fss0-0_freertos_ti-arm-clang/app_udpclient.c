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

/* THREAD 2: UDP Streaming Task (Broadcasts formatted packets) */
static void AppSocket_udpServerTask(void *pArg)
{
    int32_t sock = -1, ret = 0;
    struct sockaddr_in broadcast_addr;
    uint8_t tx_buffer[TX_BUFFER_SIZE];

    EnetAppUtils_print("UDP Broadcast Thread Started on Port %d...\r\n", SERVER_UDP_PORT);

    /* Create the socket */
    sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        EnetAppUtils_print("ERR: unable to open socket\r\n");
        vTaskDelete(NULL);
    }

    /* Enable UDP Broadcasting on this socket */
    int broadcast_enable = 1;
    ret = lwip_setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    if (ret < 0) {
        EnetAppUtils_print("ERR: Failed to enable broadcast\r\n");
    }

    /* Setup destination address: 255.255.255.255 */
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = PP_HTONS(SERVER_UDP_PORT);
    broadcast_addr.sin_addr.s_addr = PP_HTONL(INADDR_BROADCAST); 

    uint16_t sequence_num = 1;

    /* Main Streaming Loop */
    while (1)
    {
        /* Lock Mutex to safely read logs */
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            // Send up to the last 10 logs
            int logs_to_send = (gTotalLogs < 10) ? gTotalLogs : 10;
            int start_idx = (gTotalLogs < 10) ? 0 : gTotalLogs - 10;
            
            for (int i = 0; i < logs_to_send; i++)
            {
                int actual_idx = (start_idx + i) % MAX_LOGS;
                LogItem_t *log = &gLogBuffer[actual_idx];

                memset(tx_buffer, 0, sizeof(tx_buffer));

                // 1. Format Payload (Thomas's LogEntry format)
                uint8_t payload[256];
                memset(payload, 0, sizeof(payload));
                
                payload[ENTRY_STATUS] = 0; // STATUS_OK
                
                // For demonstration, padding out timestamp to look like valid 64-bit ms
                uint64_t fake_timestamp_ms = 1774890000000 + log->timestamp;
                write_u64(&payload[ENTRY_TS], fake_timestamp_ms); 
                
                payload[ENTRY_TYPE] = log->type; // 0=ERROR, 1=WARNING, 2=INFO
                
                // Format the message string
                char msg[MAX_MSG_LEN];
                snprintf(msg, sizeof(msg), "Temp: %.1fC, Volts: %.1fV", log->temperature, log->voltage);
                int msg_len = strlen(msg);
                memcpy(&payload[ENTRY_MSG], msg, msg_len);
                
                uint16_t payload_len = ENTRY_MSG + msg_len;

                // 2. Build Thomas's Header (14 bytes)
                write_u16(&tx_buffer[PKT_ID], PACKET_MAGIC);
                tx_buffer[PKT_CMD] = CMD_GET_ALL_LOGS;
                write_u16(&tx_buffer[PKT_SEQ], sequence_num);
                write_u16(&tx_buffer[PKT_TOT], logs_to_send);
                write_u16(&tx_buffer[PKT_IDX], i); // Index of this specific log
                write_u16(&tx_buffer[PKT_LEN], payload_len);
                
                // 3. Copy Payload into Packet
                memcpy(&tx_buffer[PKT_DATA], payload, payload_len);

                // 4. Calculate and Append Checksum
                int total_pkt_len = PKT_DATA + payload_len + 2;
                uint16_t checksum = calc_checksum(tx_buffer, total_pkt_len);
                write_u16(&tx_buffer[PKT_DATA + payload_len], checksum);

                // 5. Blast data to the network
                ret = lwip_sendto(sock, tx_buffer, total_pkt_len, 0,
                                  (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
            }

            if (logs_to_send > 0) {
                EnetAppUtils_print("Broadcasted Sequence %d (%d logs) to network\r\n", sequence_num, logs_to_send);
                sequence_num++;
            }

            xSemaphoreGive(gLogMutex); /* Unlock Mutex */
        }

        /* Sleep for 1 second before broadcasting the next batch */
        vTaskDelay(pdMS_TO_TICKS(1000));
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
