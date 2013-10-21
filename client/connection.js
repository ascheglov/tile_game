var Dir = {Right: 0, Up: 1, Left: 2, Down: 3};
var PlayerState = {Idle: 0, MovingOut: 1, MovingIn: 2};

var connection = (function()
{
    var this_ = {}

    var websocket;

    var log = function(text) { this_.ui.log(text); }
    var log_error = function(text) { this_.ui.log(text, 'error'); }

    var send = function(msg)
    {
        log('SEND: ' + msg);
        if (!websocket) { log_error('not connected'); return; }
        websocket.send(msg);
    }

    this_.connect = function(uri)
    {
        websocket = new WebSocket(uri);
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
        case Dir.Right: info.x += 1; break;
        case Dir.Up: info.y -= 1; break;
        case Dir.Left: info.x -= 1; break;
        case Dir.Down: info.y += 1; break;
        default: log_error('invalid "dir" attribute');
        }

        this_.ui.setPos(el, info);
        info.state = PlayerState.MovingIn;
        this_.ui.setState(el, info);
    }

    var messageHandlers =
    {
        init: function(pkt) { pkt.state = PlayerState.Idle; this_.ui.addPlayer(pkt); },
        see_player: function(pkt) { this_.ui.addPlayer(pkt); },
        disconnect: function(pkt) { log('disconnected by server'); },
        see_disappear: function(pkt) { this_.ui.removePlayer(pkt.id); },
        see_begin_move: function(pkt)
        {
            pkt.state = PlayerState.MovingOut;
            var player = this_.ui.find(pkt.id);
            this_.ui.setState(player, pkt);
        },
        see_cross_cell: movePlayer,
        see_stop: function(pkt)
        {
            pkt.state = PlayerState.Idle;
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
        if (!(dirStr in Dir)) { log_error('invalid direction: ' + dirStr); return; }

        send('move ' + Dir[dirStr]);
    };

    this_.disconnect = function()
    {
        websocket.close();
    };

    return this_;
})();