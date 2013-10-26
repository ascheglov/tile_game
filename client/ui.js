var ui = (function()
{
    var this_ = {}

    var worldCX = 8, worldCY = 8;

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

        if ('spell' in info)
            el.attr('spell', info.spell);
    };

    this_.getAttr = function(el, name)
    {
        return el.attr(name);
    };

    this_.moveView = function(info)
    {
        var distance = function(x, y)
        {
            return Math.abs(x - info.x) + Math.abs(y - info.y);
        };

        for (var y = 0; y != worldCY; y++)
        {
            var tr = $('#view tr').eq(y);
            for (var x = 0; x != worldCY; x++)
            {
                $('td', tr).eq(x).toggleClass('v', distance(x, y) <= 2);
            }
        }
    };

    this_.setMap = function(info)
    {
        var cx = info.cx;
        var cy = info.cells.length / cx;
        var view = $('#view');
        view.empty();
        
        var idx = 0;
        for (var y = 0; y != cy; y++)
        {
            var row = $('<tr>');
            for (var x = 0; x != cx; x++, idx++)
            {
                var cell = $('<td>');
                switch (info.cells[idx])
                {
                case 'W': cell.addClass('wall'); break;
                }
                row.append(cell);
            }
            view.append(row);
        }
    };
    
    this_.healthChange = function(info)
    {
        $('#hp').text(info.hp);
    };
    
    this_.addEffect = function(info)
    {
        var ofs = cellOffset(info.x, info.y);
        var effect = $('<div/>');
        effect.attr('effect', info.effect).appendTo('#effects');
        effect.css('left', ofs.left).css('top', ofs.top);
        setTimeout(function() { effect.remove(); }, 1000);
    };
    
    this_.getTargetPos = function()
    {
        var td = $('#view td.target');
        var x = td.index();
        var y = td.closest('tr').index();
        return x + ' ' + y;
    };
    
    this_.setTargetCell = function(td)
    {
        $('#view td.target').removeClass('target');
        td.addClass('target');
    };
    
    return this_;
})();

$(document).ready(function()
{
    connection.ui = ui;

    $("#btn-connect").click(function()
    {
        $('#players').empty();
        $('#view td').removeClass('v');

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

    $("#btn-cast-lightning").click(function()
    {
        connection.requestCast(Spell.Lightning, ui.getTargetPos());
    });

    $("#btn-cast-self-heal").click(function()
    {
        connection.requestCast(Spell.SelfHeal, '');
    });
    
    $("#view").on('click', 'td', function(e)
    {
        ui.setTargetCell($(this));
    });

    $("#players").on('click', 'div', function(e)
    {
        ui.setTargetCell($('#view tr:eq(' + $(this).attr('y') + ') td:eq(' + $(this).attr('x') + ')'));
    });
    
    ui.log('initialization finished');
});