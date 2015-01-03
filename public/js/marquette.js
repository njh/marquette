var columns = 4;
var rows = 4;
var min_margin = 6;
var textfill_config = {
    innerTag: 'span',
    minFontPixels: 4,
    maxFontPixels: 0
};

$.postJSON = function(url, data, callback) {
    return jQuery.ajax({
        'type': 'POST',
        'url': url,
        'contentType': 'application/json',
        'data': $.toJSON(data),
        'dataType': 'json',
        'success': callback
    });
};

$(function(){ //DOM Ready

    function serialise_params(tile, wgd)
    {
        var params = { row: wgd.row, col: wgd.col };
        var blacklist = ['row', 'col', 'coords', 'sizex', 'sizey'];
        for(var key in tile.data()) {
            if (blacklist.indexOf(key) < 0) {
                params[key] = tile.data(key);
            }
        }
        return params;
    }

    function init_gridster()
    {
        var win = $( window );

        var tile_size_x = Math.floor((win.width() - (columns * min_margin * 2) - min_margin) / columns);
        if (tile_size_x % 2 == 1)
            tile_size_x -= 1;

        var height = win.height() - $('#footer').height();
        var tile_size_y = Math.floor((height - (rows * min_margin * 2) - min_margin) / rows);
        if (tile_size_y % 2 == 1)
            tile_size_y -= 1;

        var tile_size = Math.min(tile_size_x, tile_size_y);
        var margin_x = Math.floor((win.width() - (tile_size * columns)) / (columns*2));
        var margin_y = Math.floor((height - (tile_size * rows)) / (rows*2));

        var gridster = $(".gridster").gridster({
            widget_selector: ".tile",
            widget_base_dimensions: [tile_size, tile_size],
            widget_margins: [margin_x, margin_y],
            serialize_params: serialise_params,
            min_cols: columns,
            max_cols: columns
        }).data('gridster');

        // Disable dragging
        gridster.disable();

        return gridster;
    }

    var gridster = init_gridster();

    $.getJSON("tiles",function(tiles) {
        $.each(tiles, function(i, tile) {
            var div = $("<div></div>").attr('id', "tile"+i).addClass('tile');
            $.each(tile, function(k, v) { div.data(k, v) });

            if (tile.type == 'button') {
                var button = $('<button></button>').attr('type', 'button');
                button.on("click", function(event) {
                    $.postJSON('/topics/'+tile.publish_topic, {payload: tile.publish_payload});
                });
                button.appendTo(div);
                div.append("<label>"+tile.name+"</label>");
            } else if (tile.type == 'text') {
                div.append('<div class="text"><span></span></div>');
                div.append('<label>'+tile.name+'</label>');
            } else {
                div.html("Unknown tile type: "+tile.type);
            }

            gridster.add_widget(div, size_x=1, size_y=1, tile.col, tile.row);
        });
   });

    function save_tiles(button) {
        var tile_data = gridster.serialize();
        $.postJSON(
            'tiles', tile_data
        ).done(function(data, textStatus) {
            $( '.tile' ).removeClass('editable');
            gridster.disable();
            button.html('Edit...');
        });
    }

    $("#edit").on("click", function(event) {
        var button = $( event.target );
        if (button.html() == 'Edit...') {
            $( '.tile' ).addClass('editable');
            gridster.enable();
            button.html('Save');
        } else {
            save_tiles(button);
        }
    });
    

    $( window ).resize(function() {
        var gridster = $('.gridster').data('gridster');
        if (gridster) {
            gridster.destroy();
        }
        init_gridster();
        $('.text').textfill(textfill_config);
    });

    FastClick.attach(document.body);

    var source = new EventSource('/update-stream');
    console.log("EventSource="+source);

    source.onopen = function(e) {
        console.log("EventSource connection open");
    };

    source.onerror = function(e) {
        console.log("Event source error: "+e);
    };

    source.onmessage = function(e) {
        var obj = JSON.parse(e.data);
        $('.tile').each(function() {
            var tile = $(this);
            if (tile.data('subscribe_topic') == obj.topic) {
                var text = tile.find('.text');
                if (text) {
                    text.html('<span>'+obj.payload+'</span>');
                    text.textfill(textfill_config);
                }
            }
        });
    };


