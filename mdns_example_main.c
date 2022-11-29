/* MDNS-SD Query and advertise Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif_ip_addr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "mdns.h"
#include "driver/gpio.h"
#include "netdb.h"
#include <sys/socket.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "coap3/coap.h"

#define EXAMPLE_MDNS_INSTANCE CONFIG_MDNS_INSTANCE

#define SHOELACE_URI        "shoe/shoelace" 
#define LEDCOLOR_URI        "shoe/ledcolor" 
#define STEPS_URI           "shoe/steps" 
#define SIZE_URI            "shoe/size" 
#define NAME_URI            "shoe/name" 
#define LACES_TIE           "tie" 
#define LACES_UNTIE         "untie"
#define LEDCOLOR_DEFAULT    "000000"
#define STEPS_DEFAULT       "0"
#define NAME_DEFAULT        "Empty"
#define SIZE                "7.5MX"

static const char * TAG = "Practica_3_CoAP_mDNS";
static char * generate_hostname(void);

#define EXAMPLE_COAP_LOG_DEFAULT_LEVEL CONFIG_COAP_LOG_DEFAULT_LEVEL

#define INITIAL_DATA "Hello World!"

static char espressif_data[100];
static int espressif_data_len = 0;
static char shoelace[100];
static int shoelace_len = 0;
static char ledcolor[100];
static int ledcolor_len = 0;
static char steps[100];
static int steps_len = 0;
static char size[100];
static int size_len = 0;
static char name[100];
static int name_len = 0;
static int steps_number = 0;

static void initialise_mdns(void)
{
    char * hostname = generate_hostname();

    //initialize mDNS
    ESP_ERROR_CHECK( mdns_init() );
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK( mdns_hostname_set(hostname) );
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
    //set default mDNS instance name
    ESP_ERROR_CHECK( mdns_instance_name_set(EXAMPLE_MDNS_INSTANCE) );

    //structure with TXT records
    mdns_txt_item_t serviceTxtData[3] = {
        {"board", "esp32"},
        {"u", "user"},
        {"p", "password"}
    };

    //initialize service
    ESP_ERROR_CHECK( mdns_service_add("shoe_control", "_coap", "_udp", 80, serviceTxtData, 3) );
    //add another TXT item
    ESP_ERROR_CHECK( mdns_service_txt_item_set("_coap", "_udp", "path", "/foobar") );
    //change TXT item value
    ESP_ERROR_CHECK( mdns_service_txt_item_set_with_explicit_value_len("_coap", "_udp", "u", "admin", strlen("admin")) );
    free(hostname);
}


/*
 * The resource handler
 */
static void
hnd_espressif_get(coap_resource_t *resource,
                  coap_session_t *session,
                  const coap_pdu_t *request,
                  const coap_string_t *query,
                  coap_pdu_t *response)
{
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data_large_response(resource, session, request, response,
                                 query, COAP_MEDIATYPE_TEXT_PLAIN, 60, 0,
                                 (size_t)espressif_data_len,
                                 (const u_char *)espressif_data,
                                 NULL, NULL);
}

static void
hnd_espressif_put(coap_resource_t *resource,
                  coap_session_t *session,
                  const coap_pdu_t *request,
                  const coap_string_t *query,
                  coap_pdu_t *response)
{
    size_t size;
    size_t offset;
    size_t total;
    const unsigned char *data;
    

    if (strcmp (espressif_data, INITIAL_DATA) == 0) {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CREATED);
    } else {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED);
    }

    /* coap_get_data_large() sets size to 0 on error */
    (void)coap_get_data_large(request, &size, &data, &offset, &total);


    if (size == 0) {      /* re-init */
        snprintf(espressif_data, sizeof(espressif_data), INITIAL_DATA);
        espressif_data_len = strlen(espressif_data);
    } else {
        espressif_data_len = size > sizeof (espressif_data) ? sizeof (espressif_data) : size;
        memcpy (espressif_data, data, espressif_data_len);
    }
}

static void
hnd_espressif_delete(coap_resource_t *resource,
                     coap_session_t *session,
                     const coap_pdu_t *request,
                     const coap_string_t *query,
                     coap_pdu_t *response)
{
    snprintf(espressif_data, sizeof(espressif_data), INITIAL_DATA);
    espressif_data_len = strlen(espressif_data);
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_DELETED);
}

/* Practica 3 - Recursos */

/* URI: shoe/shoelace--------------------------------------------------------------------------------- */

static void
hnd_shoelace_put(coap_resource_t *resource,
                  coap_session_t *session,
                  const coap_pdu_t *request,
                  const coap_string_t *query,
                  coap_pdu_t *response)
{
    size_t size;
    size_t offset;
    size_t total;
    const unsigned char *data;
    

    if (strcmp (shoelace, LACES_UNTIE) == 0) {
         coap_pdu_set_code(response, COAP_RESPONSE_CODE_CREATED);
    } else {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED);
    }

    /* coap_get_data_large() sets size to 0 on error */
    (void)coap_get_data_large(request, &size, &data, &offset, &total);

    
    if (size == 0) {     /* re-init */
        snprintf(shoelace, sizeof(shoelace), LACES_UNTIE);
        shoelace_len = strlen(shoelace);
    } else {
        shoelace_len = size > sizeof (shoelace) ? sizeof (shoelace) : size;
        memcpy (shoelace, data, shoelace_len);
    }
    
}

static void
hnd_shoelace_get(coap_resource_t *resource,
                  coap_session_t *session,
                  const coap_pdu_t *request,
                  const coap_string_t *query,
                  coap_pdu_t *response)
{
   

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data_large_response(resource, session, request, response,
                                 query, COAP_MEDIATYPE_TEXT_PLAIN, 60, 0,
                                 (size_t)shoelace_len,
                                 (const u_char *)shoelace,
                                 NULL, NULL);
}


/* URI: shoe/ledcolor--------------------------------------------------------------------------------- */

static void
hnd_ledcolor_put(coap_resource_t *resource,
                  coap_session_t *session,
                  const coap_pdu_t *request,
                  const coap_string_t *query,
                  coap_pdu_t *response)
{
    size_t size;
    size_t offset;
    size_t total;
    const unsigned char *data;
    

    if (strcmp (ledcolor, LEDCOLOR_DEFAULT) == 0) {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CREATED);
    } else {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED);
    }

    /* coap_get_data_large() sets size to 0 on error */
    (void)coap_get_data_large(request, &size, &data, &offset, &total);


    if (size == 0) {      /* re-init */
        snprintf(ledcolor, sizeof(ledcolor), LEDCOLOR_DEFAULT);
        ledcolor_len = strlen(ledcolor);
    } else {
        ledcolor_len = size > sizeof (ledcolor) ? sizeof (ledcolor) : size;
        memcpy (ledcolor, data, ledcolor_len);
    }
}

static void
hnd_ledcolor_get(coap_resource_t *resource,
                  coap_session_t *session,
                  const coap_pdu_t *request,
                  const coap_string_t *query,
                  coap_pdu_t *response)
{


    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data_large_response(resource, session, request, response,
                                 query, COAP_MEDIATYPE_TEXT_PLAIN, 60, 0,
                                 (size_t)ledcolor_len,
                                 (const u_char *)ledcolor,
                                 NULL, NULL);
}


static void
hnd_ledcolor_delete(coap_resource_t *resource,
                    coap_session_t *session,
                    const coap_pdu_t *request,
                    const coap_string_t *query,
                    coap_pdu_t *response)
{
    snprintf(ledcolor, sizeof(ledcolor), LEDCOLOR_DEFAULT);
    ledcolor_len = strlen(ledcolor);
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_DELETED);
}

/* URI: shoe/steps--------------------------------------------------------------------------------- */
static void
hnd_steps_get(coap_resource_t *resource,
              coap_session_t *session,
              const coap_pdu_t *request,
              const coap_string_t *query,
              coap_pdu_t *response)
{
    
    sprintf(steps, "%d", steps_number);
    steps_len = strlen(steps);

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data_large_response(resource, session, request, response,
                                 query, COAP_MEDIATYPE_TEXT_PLAIN, 60, 0,
                                 (size_t)steps_len,
                                 (const u_char *)steps,
                                 NULL, NULL);
}

static void
hnd_steps_delete(coap_resource_t *resource,
                 coap_session_t *session,
                 const coap_pdu_t *request,
                 const coap_string_t *query,
                 coap_pdu_t *response)
{
    steps_number = 0;
    snprintf(steps, sizeof(steps), STEPS_DEFAULT);
    steps_len = strlen(steps);
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_DELETED);
    
}

/* URI: shoe/size--------------------------------------------------------------------------------- */
static void
hnd_size_get(coap_resource_t *resource,
             coap_session_t *session,
             const coap_pdu_t *request,
             const coap_string_t *query,
             coap_pdu_t *response)

{

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data_large_response(resource, session, request, response,
                                 query, COAP_MEDIATYPE_TEXT_PLAIN, 60, 0,
                                 (size_t)size_len,
                                 (const u_char *)size,
                                 NULL, NULL);
}

/* URI: shoe/name--------------------------------------------------------------------------------- */

static void
hnd_name_put(coap_resource_t *resource,
             coap_session_t *session,
             const coap_pdu_t *request,
             const coap_string_t *query,
             coap_pdu_t *response)
{
    size_t size;
    size_t offset;
    size_t total;
    const unsigned char *data;

    if (strcmp (name, NAME_DEFAULT) == 0) {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CREATED);
    } else {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED);
    }

    /* coap_get_data_large() sets size to 0 on error */
    (void)coap_get_data_large(request, &size, &data, &offset, &total);


    if (size == 0) {      /* re-init */
        snprintf(name, sizeof(name), NAME_DEFAULT);
        name_len = strlen(name);
    } else {
        name_len = size > sizeof (name) ? sizeof (name) : size;
        //memcpy (name, data, name_len);
        strncpy(name, (const char *)data, 100);
    }
}

static void
hnd_name_get(coap_resource_t *resource,
             coap_session_t *session,
             const coap_pdu_t *request,
             const coap_string_t *query,
             coap_pdu_t *response)
{
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data_large_response(resource, session, request, response,
                                 query, COAP_MEDIATYPE_TEXT_PLAIN, 60, 0,
                                 (size_t)name_len,
                                 (const u_char *)name,
                                 NULL, NULL);
}

static void
hnd_name_delete(coap_resource_t *resource,
                coap_session_t *session,
                const coap_pdu_t *request,
                const coap_string_t *query,
                coap_pdu_t *response)
{
    snprintf(name, sizeof(name), NAME_DEFAULT);
    name_len = strlen(name);
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_DELETED);
}




void
coap_log_handler (coap_log_t level, const char *message)
{
    uint32_t esp_level = ESP_LOG_INFO;
    char *cp = strchr(message, '\n');

    if (cp)
        ESP_LOG_LEVEL(esp_level, TAG, "%.*s", (int)(cp-message), message);
    else
        ESP_LOG_LEVEL(esp_level, TAG, "%s", message);
}

static void coap_example_server(void *p)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;
    coap_resource_t *resource = NULL;
    coap_resource_t *resource_shoelace = NULL;
    coap_resource_t *resource_ledcolor = NULL;
    coap_resource_t *resource_steps = NULL;
    coap_resource_t *resource_size = NULL;
    coap_resource_t *resource_name = NULL;

    
    snprintf(espressif_data, sizeof(espressif_data), INITIAL_DATA);
    espressif_data_len = strlen(espressif_data);
    snprintf(shoelace, sizeof(shoelace), LACES_UNTIE);
    snprintf(ledcolor, sizeof(ledcolor), LEDCOLOR_DEFAULT);
    snprintf(steps, sizeof(steps), STEPS_DEFAULT);
    snprintf(size, sizeof(size), SIZE);
    size_len = strlen(size);
    snprintf(name, sizeof(name), NAME_DEFAULT);
    
    coap_set_log_handler(coap_log_handler);
    coap_set_log_level(EXAMPLE_COAP_LOG_DEFAULT_LEVEL);

    ESP_LOGI(TAG, "CoAP server example started!");

    while (1) {
        coap_endpoint_t *ep = NULL;
        unsigned wait_ms;

        /* Prepare the CoAP server socket */
        coap_address_init(&serv_addr);
        serv_addr.addr.sin6.sin6_family = AF_INET6;
        serv_addr.addr.sin6.sin6_port   = htons(COAP_DEFAULT_PORT);

        ctx = coap_new_context(NULL);
        if (!ctx) {
            ESP_LOGE(TAG, "coap_new_context() failed");
            continue;
        }
        coap_context_set_block_mode(ctx,
                                    COAP_BLOCK_USE_LIBCOAP|COAP_BLOCK_SINGLE_BODY);

        ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
        if (!ep) {
            ESP_LOGE(TAG, "udp: coap_new_endpoint() failed");
            goto clean_up;
        }
        ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_TCP);
        if (!ep) {
            ESP_LOGE(TAG, "tcp: coap_new_endpoint() failed");
            goto clean_up;
        }

        
        resource = coap_resource_init(coap_make_str_const("Espressif"), 0);
        if (!resource) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }
        coap_register_handler(resource, COAP_REQUEST_GET, hnd_espressif_get);
        coap_register_handler(resource, COAP_REQUEST_PUT, hnd_espressif_put);
        coap_register_handler(resource, COAP_REQUEST_DELETE, hnd_espressif_delete);
        coap_add_resource(ctx, resource);

        /* Init de shoelace*/
        resource_shoelace = coap_resource_init(coap_make_str_const(SHOELACE_URI), 0);
        if (!resource_shoelace) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }
        coap_register_handler(resource_shoelace, COAP_REQUEST_PUT, hnd_shoelace_put);
        coap_register_handler(resource_shoelace, COAP_REQUEST_GET, hnd_shoelace_get);
        coap_add_resource(ctx, resource_shoelace);

        /* Init de ledcolor*/
        resource_ledcolor = coap_resource_init(coap_make_str_const(LEDCOLOR_URI), 0);
        if (!resource_ledcolor) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }
        coap_register_handler(resource_ledcolor, COAP_REQUEST_PUT, hnd_ledcolor_put);
        coap_register_handler(resource_ledcolor, COAP_REQUEST_GET, hnd_ledcolor_get);
        coap_register_handler(resource_ledcolor, COAP_REQUEST_DELETE, hnd_ledcolor_delete);
        coap_add_resource(ctx, resource_ledcolor);

         /* Init de steps*/
        resource_steps = coap_resource_init(coap_make_str_const(STEPS_URI), 0);
        if (!resource_steps) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }
        coap_register_handler(resource_steps, COAP_REQUEST_GET, hnd_steps_get);
        coap_register_handler(resource_steps, COAP_REQUEST_DELETE, hnd_steps_delete);
        coap_add_resource(ctx, resource_steps);

        /* Init de size*/
        resource_size = coap_resource_init(coap_make_str_const(SIZE_URI), 0);
        if (!resource_size) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }
        coap_register_handler(resource_size, COAP_REQUEST_GET, hnd_size_get);
        coap_add_resource(ctx, resource_size);

        /* Init de name*/
        resource_name = coap_resource_init(coap_make_str_const(NAME_URI), 0);
        if (!resource_name) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }
        coap_register_handler(resource_name, COAP_REQUEST_PUT, hnd_name_put);
        coap_register_handler(resource_name, COAP_REQUEST_GET, hnd_name_get);
        coap_register_handler(resource_name, COAP_REQUEST_DELETE, hnd_name_delete);
        coap_add_resource(ctx, resource_name);
        

        wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;

        while (1) {
            steps_number ++;
            int result = coap_io_process(ctx, wait_ms);
            if (result < 0) {
                break;
            } else if (result && (unsigned)result < wait_ms) {
                /* decrement if there is a result wait time returned */
                wait_ms -= result;
            }
            if (result) {
                /* result must have been >= wait_ms, so reset wait_ms */
                wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
            }

            vTaskDelay(50 / portTICK_PERIOD_MS);
            
        }
    }
clean_up:
    coap_free_context(ctx);
    coap_cleanup();

    vTaskDelete(NULL);
}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    initialise_mdns();

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(&coap_example_server, "coap_example_server",  8 * 1024, NULL, 5, NULL);
}

/** Generate host name based on sdkconfig, optionally adding a portion of MAC address to it.
 *  @return host name string allocated from the heap
 */
static char* generate_hostname(void)
{
#ifndef CONFIG_MDNS_ADD_MAC_TO_HOSTNAME
    return strdup(CONFIG_MDNS_HOSTNAME);
#else
    uint8_t mac[6];
    char   *hostname;
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (-1 == asprintf(&hostname, "%s-%02X%02X%02X", CONFIG_MDNS_HOSTNAME, mac[3], mac[4], mac[5])) {
        abort();
    }
    return hostname;
#endif
}
