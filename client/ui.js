var ui = (function()
{
    var this_ = {}

    var cellOffset = function(x, y)
    {
        return $('#view tr:eq(' + y + ') td:eq(' + x + ')').offset();
    };

    this_.log = function(text, level)
    {
        var entry = $('<p>').text(text);
        if (level)
            entry.addClass(level);
        entry.appendTo('#log');
    };

    this_.find = function(id)
    {
        return $('#obj_' + id);
    };

    this_.addPlayer = function(info)
    {
        var player = $('<div/>');
        player.attr('id', 'obj_' + info.id).text(info.id).appendTo('#players');
        this_.setPos(player, info);
        this_.setState(player, info);
    };

    this_.removePlayer = function(id)
    {
        this_.find(id).remove();
    };

    this_.setPos = function(el, info)
    {
        var ofs = cellOffset(info.x, info.y);
        el.css('left', ofs.left).css('top', ofs.top);
        el.attr('x', info.x).attr('y', info.y);
    };

    this_.setState = function(el, info)
    {
        el.attr('state', info.state);
        if ('dir' in info)
            el.attr('dir', info.dir);
    };

    this_.getAttr = function(el, name)
    {
        return el.attr(name);
    };

    return this_;
})();

$(document).ready(function()
{
    connection.ui = ui;

    $("#btn-connect").click(function()
    {
        $('#players').empty();
        connection.connect($('#server-uri').val());
    });

    $("#btn-disconnect").click(function()
    {
        connection.disconnect();
    });

    $("#btn-test").click(function()
    {
        connection.onMessage($('#test-msg').val());
    });

    $("#btn-clear-log").click(function()
    {
        $('#log').empty();
    });

    $(".btn-move").click(function()
    {
        connection.requestMove($(this).attr('dir'));
    });

    ui.log('initialization finished');
});