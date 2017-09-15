#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_platform.h>
#include <artik_gpio.h>
#include <artik_mqtt.h>

#define MAX_MSG_LEN 512
#define MAX_UUID_LEN    128

artik_gpio_module *gpio = NULL;

struct motor_gpio {
	artik_gpio_handle handle;
	artik_gpio_config config;
};

struct motor_gpio motor_port[] = {
		{ NULL,{ ARTIK_A710_GPIO0, "Motor1_L",GPIO_OUT,GPIO_EDGE_NONE, 0, NULL } },
		{ NULL,{ ARTIK_A710_GPIO1, "Motor1_R",GPIO_OUT,GPIO_EDGE_NONE, 0, NULL } },
		{ NULL,{ ARTIK_A710_GPIO2, "Motor2_L",GPIO_OUT,GPIO_EDGE_NONE, 0, NULL } },
		{ NULL,{ ARTIK_A710_GPIO3, "Motor2_R",GPIO_OUT,GPIO_EDGE_NONE, 0, NULL } },
};

static artik_mqtt_module *mqtt = NULL;
static artik_loop_module *loop = NULL;

static char device_id[MAX_UUID_LEN] = "7b2a64626f8f4b5caac2a3fee8e4adc2";
static char token[MAX_UUID_LEN] = "e65c8e75d365462e8fdf6069685c146d";
static char pub_msg[MAX_MSG_LEN] = "";
static const char *akc_root_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\r\n"
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\r\n"
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\r\n"
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\r\n"
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\r\n"
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\r\n"
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\r\n"
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\r\n"
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\r\n"
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\r\n"
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\r\n"
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\r\n"
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\r\n"
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\r\n"
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\r\n"
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\r\n"
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\r\n"
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\r\n"
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\r\n"
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\r\n"
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\r\n"
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\r\n"
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\r\n"
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\r\n"
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\r\n"
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\r\n"
"-----END CERTIFICATE-----\n";


void control_motor(int select){
    if (select == 1) // 전진
    {
        printf("go front ----->\n");
        gpio->write(motor_port[0].handle, 1);
        gpio->write(motor_port[1].handle, 0);
        gpio->write(motor_port[2].handle, 1);
        gpio->write(motor_port[3].handle, 0);
    }
    else if (select == 2) // 왼쪽
    {
        printf("go left ----->\n");
        gpio->write(motor_port[0].handle, 0);
        gpio->write(motor_port[1].handle, 1);
        gpio->write(motor_port[2].handle, 1);
        gpio->write(motor_port[3].handle, 0);
    }
    else if (select == 3) // 오른쪽
    {
        printf("go right ----->\n");
        gpio->write(motor_port[0].handle, 1);
        gpio->write(motor_port[1].handle, 0);
        gpio->write(motor_port[2].handle, 0);
        gpio->write(motor_port[3].handle, 1);
    }
    else if (select == 4) // 후진
    {
        printf("go back ----->\n");
        gpio->write(motor_port[0].handle, 0);
        gpio->write(motor_port[1].handle, 1);
        gpio->write(motor_port[2].handle, 0);
        gpio->write(motor_port[3].handle, 1);
    }
    else if (select == 5) // 정지
    {
        printf("stop ----->\n");
        gpio->write(motor_port[0].handle, 0);
        gpio->write(motor_port[1].handle, 0);
        gpio->write(motor_port[2].handle, 0);
        gpio->write(motor_port[3].handle, 0);
    }
}

void on_connect_subscribe(artik_mqtt_config *client_config, void *user_data, artik_error result)
{
    printf("-----on_connect_subscribe-----\n");
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)client_config->handle;
	artik_mqtt_msg *msg = (artik_mqtt_msg *)user_data;

	char pub_topic[MAX_UUID_LEN + 128];

	artik_error ret;

	if (result == S_OK && client_data) {
		/* Subscribe to receive actions */
		ret = mqtt->subscribe(client_data, msg->qos, msg->topic);
        // 메인문에서 input값으로 입력받은 client_config, user_data(msg)를 이용하여 subscribe함수 호출
		if (ret == S_OK)
			fprintf(stdout, "subscribe success\n");
		else
			fprintf(stderr, "subscribe err: %s\n", error_msg(ret));
	}
}

void on_message_disconnect(artik_mqtt_config *client_config, void *user_data,artik_mqtt_msg *msg)
{
    printf("-----on_message_disconnect-----\n");
	artik_mqtt_handle *client_data;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *)user_data;
	char receive_data[100];
	int select;

	if (msg && client_config) {
	    char *str = (char *)msg->payload;
        strcpy(receive_data,(char *)msg->payload);
        memcpy(receive_data,&str[63], 2);
        receive_data[2] = 0;
        select = atoi(receive_data);
		fprintf(stdout, "content : %d\n", select);
		if(select != -1){
            control_motor(select);
		}
		else{
            client_data = (artik_mqtt_handle *)client_config -> handle;
            user_mqtt->disconnect(client_data);
		}
	}
}

void on_disconnect(artik_mqtt_config *client_config, void *user_data,artik_error result)
{
    printf("-----on_disconnect-----\n");
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)client_config->handle;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *)user_data;
	if (client_data) {
		user_mqtt->destroy_client(client_data);
		client_data = NULL;
		loop->quit();
	}
}

void on_publish(artik_mqtt_config *client_config, void *user_data, int result)
{
    printf("-----on_publish-----\n");
	fprintf(stdout, "message published (%d)\n", result);
}

int main() {
	int i;
	int select;

	int broker_port = 8883;
	char sub_topic[MAX_UUID_LEN + 128];
	artik_mqtt_config config;
	artik_mqtt_msg subscribe_msg;
	artik_mqtt_handle client;
	artik_ssl_config ssl;

	gpio = (artik_gpio_module *)artik_request_api_module("gpio");
	artik_error ret = S_OK;

	ret = gpio->request(&motor_port[0].handle, &motor_port[0].config);
	ret = gpio->request(&motor_port[1].handle, &motor_port[1].config);
	ret = gpio->request(&motor_port[2].handle, &motor_port[2].config);
	ret = gpio->request(&motor_port[3].handle, &motor_port[3].config);

	mqtt = (artik_mqtt_module *)artik_request_api_module("mqtt");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	memset(&subscribe_msg, 0, sizeof(artik_mqtt_msg));

	snprintf(sub_topic, sizeof(sub_topic), "/v1.1/actions/%s", device_id);
	subscribe_msg.topic = sub_topic;
	subscribe_msg.qos = 0;
	memset(&config, 0, sizeof(artik_mqtt_config));
	config.client_id = "sub_client";
	config.block = true;
	config.user_name = device_id;
	config.pwd = token;

	/* TLS configuration  */
	memset(&ssl, 0, sizeof(artik_ssl_config));
	ssl.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
	ssl.ca_cert.data = (char *)akc_root_ca;
	ssl.ca_cert.len = strlen(akc_root_ca);
	config.tls = &ssl;

	/* Connect to server */
	mqtt->create_client(&client, &config);

	mqtt->set_connect(client, on_connect_subscribe, &subscribe_msg);
	mqtt->set_disconnect(client, on_disconnect, mqtt);
	mqtt->set_publish(client, on_publish, mqtt);
	mqtt->set_message(client, on_message_disconnect, mqtt);

	mqtt->connect(client, "api.artik.cloud", broker_port);

	loop->run();
	// loop -> quit()함수를 만날때 종료.

	printf("loop finish\n");

	/* Release api modules */


    artik_release_api_module(mqtt);
	artik_release_api_module(loop);
	gpio->release(motor_port[0].handle);
	gpio->release(motor_port[1].handle);
	gpio->release(motor_port[2].handle);
	gpio->release(motor_port[3].handle);
	ret = artik_release_api_module(gpio);
	return 0;
}
