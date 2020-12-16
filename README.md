# A minimal client of alpaca.markets streaming API written in C.



## Compilation:

```
gcc -lwebsockets alpaca-streaming-client.c
```

Make sure libwebsockets is present in the system before compiling.

## Remarks
Alpaca key and secret are hardcoded for simplicity. So is the ticker "SPY" for which the streaming data is being read.

## See also
Alpaca market data streaming:
[https://alpaca.markets/docs/api-documentation/api-v2/market-data/streaming/](https://alpaca.markets/docs/api-documentation/api-v2/market-data/streaming/)

Libwebsockets:
[https://libwebsockets.org](https://libwebsockets.org)

The WebSocket protocol, RFC 6455:
[https://tools.ietf.org/html/rfc6455](https://tools.ietf.org/html/rfc6455)


