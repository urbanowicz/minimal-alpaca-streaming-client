/*
 * alpaca-streaming-client.c
 *
 * Tomasz Urbanowicz <tomasz.urbanowicz@gmail.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * A websocket client that connects to Alpaca's data streaming api.
 */

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

/* Possible states of the program. */
enum State {NotConnected=0, ConnectionEstablished=1, Authorized=2, Listening=3};
static enum State state = NotConnected;

/* Our websocket. */
static struct lws *websocket;

/* Connect to 'data.alpaca.markets/stream'. */
static const char *server_address = "data.alpaca.markets";
static const char *path = "/stream";
static int port = 443;

/* Using ws://  */
static int ssl_connection = LCCSCF_USE_SSL;

/* Need to specify a protocol. RFC 6455 is using 'chat' and so are we. */ 
static const char *protocol = "chat";

/* Initiates the connection. */
static void connect_client(struct lws_context *context)
{
    struct lws_client_connect_info i;

    memset(&i, 0, sizeof(i));

    i.context = context;
    i.port = port;
    i.address = server_address;
    i.path = path;
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = ssl_connection;
    i.protocol = protocol;
    i.local_protocol_name = protocol;
    i.pwsi = &websocket;

    if (!lws_client_connect_via_info(&i))
    {
        lwsl_err("%s: Connection attempt failed.\n", __func__);
    }
}

/* Libwebsockets will call this callback function whenever something interesting happens with our connection. */
static int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    const char *authenticate_msg = "{\"action\":\"authenticate\", \"data\":{ \"key_id\":\"<ALPACA_KEY>\",\"secret_key\":\"<ALPACA_SECRET>\"}}";
    const char *listen_msg = "{\"action\":\"listen\",\"data\":{\"streams\":[\"AM.SPY\"]}}";
    const char *msg = NULL;
    size_t msg_len = 0;
    char buf[LWS_PRE + 1024];

    switch (reason) {

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char *)in : "(null)");
        state = NotConnected;
        break;
    
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        lwsl_user("CLIENT_ESTABLISHED\n");
        state = ConnectionEstablished;
        lws_callback_on_writable(wsi);
        break;
    
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        lwsl_user("CLIENT_WRITEABLE\n");
        if (state == ConnectionEstablished) 
        {
            msg = authenticate_msg;
        } 
        else
        if (state == Authorized) {
            msg = listen_msg;
        }
        else 
        {
            break;
        }
        msg_len = strlen(msg);
        memset(&buf[LWS_PRE], 0, 1024);
        strncpy(&buf[LWS_PRE], msg, msg_len);
        lws_write(wsi, (unsigned char *) &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        lwsl_user("RECEIVED: %s\n", (char *) in);
        if (state == ConnectionEstablished)
        {
            state = Authorized;
            lws_callback_on_writable(wsi);
        } 
        else
        if (state == Authorized)
        {
            state = Listening;
        }
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        lwsl_user("CONNECTION_CLOSED\n");
        state = NotConnected;
        break;

    default:
        break;
    }

    return 0;
}

static const struct lws_protocols protocols[] = {
    { "chat", callback_minimal, 0, 0, },
    { NULL, NULL, 0, 0 }
};

int main(int argc, const char **argv)
{
    struct lws_context *context;
    struct lws_context_creation_info info;

    memset(&info, 0, sizeof info);

    lwsl_user("Testing alapaca.markets streaming API.\n");

    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        lwsl_err("lws init failed\n");
        return 1;
    }
    connect_client(context);

    while (websocket != NULL)
    {
        lws_service(context, 0);
    }

    lws_context_destroy(context);
    lwsl_user("Completed\n");

    return 0;
}
