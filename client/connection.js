var connection = (function()
{
    var this_ = {}

    var SERVER_URI = "ws://localhost:4080/";

    var websocket;

    var log = function(text) { this_.ui.log(text); }
    var log_error = function(text) { this_.ui.log(text, 'error'); }

    var send = function(msg)
    {
        log('SEND: ' + msg);
        if (!websocket) { log_error('not connected'); return; }
        websocket.send(msg);
    }

    this_.connect = function()
    {
        websocket = new WebSocket(SERVER_URI);
        websocket.onopen = function(evt) { log("CONNECTED"); };
        websocket.onclose = function(evt)
        {
            log_error("DISCONNECTED");
            websocket = null;
        };
        websocket.onmessage = function(evt) { this_.onMessage(evt.data); };
        websocket.onerror = function(evt) { log_error('ERROR: ' + evt.data); };
    };

    var movePlayer = function(info)
    {
        var el = this_.ui.find(info.id);
        info.x = +this_.ui.getAttr(el, 'x');
        info.y = +this_.ui.getAttr(el, 'y');
        info.dir = +this_.ui.getAttr(el, 'dir')

        switch (info.dir)
        {
        case 0: info.x += 1; break;
        case 1: info.y -= 1; break;
        case 2: info.x -= 1; break;
        case 3: info.y += 1; break;
        default: log_error('invalid "dir" attribute');
        }

        this_.ui.setPos(el, info);
        info.state = 2;
        this_.ui.setState(el, info);
    }

    var messageHandlers =
    {
        init: function(pkt) { pkt.state = 0; this_.ui.addPlayer(pkt); },
        see_player: function(pkt) { this_.ui.addPlayer(pkt); },
        disconnect: function(pkt) { log('disconnected by server'); },
        see_disappear: function(pkt) { this_.ui.removePlayer(pkt.id); },
        see_begin_move: function(pkt)
        {
            pkt.state = 1;
            var player = this_.ui.find(pkt.id);
            this_.ui.setState(player, pkt);
        },
        see_cross_cell: movePlayer,
        see_stop: function(pkt)
        {
            pkt.state = 0;
            var player = this_.ui.find(pkt.id);
            this_.ui.setState(player, pkt);
        },
    };

    this_.onMessage = function(msg)
    {
        log('RECV: ' + msg);
        try
        {
            var pkt = JSON.parse(msg);
        }
        catch (e)
        {
            log_error('invalid packet format');
            return;
        }

        if ('type' in pkt && pkt.type in messageHandlers)
            messageHandlers[pkt.type](pkt);
        else
            log_error('unknown packet type');
    };

    this_.requestMove = function(dirStr)
    {
        var str2dir = {right: 0, up: 1, left: 2, down: 3};

        if (!(dirStr in str2dir)) { log_error('invalid direction: ' + dirStr); return; }

        send('move ' + str2dir[dirStr]);
    };

    this_.disconnect = function()
    {
        websocket.close();
    };

    return this_;
})();