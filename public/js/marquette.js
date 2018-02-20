var textfill_config = {
    innerTag: 'span',
    minFontPixels: 6,
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

    $.getJSON("tiles",function(tiles) {
        var container = $('#tiles');
        $.each(tiles, function(i, tile) {
            var item = $("<li></li>").attr('id', "tile"+i).addClass('tile');
            $.each(tile, function(k, v) { item.data(k, v) });
            $("<h4/>").append(tile.title).appendTo(item);

            if (tile.type == 'button') {
                var button = $('<button/>').attr('type', 'button').append('Press');
                button.on("click", function(event) {
                    $.postJSON('/topics/'+tile.publish_topic, {payload: tile.publish_payload});
                });
                button.appendTo(item);
            } else if (tile.type == 'text') {
                item.append('<div class="text"><span></span></div>');
            } else {
                item.html("Unknown tile type: "+tile.type);
            }
            
            $("<button />").addClass('edit').append('Edit').appendTo(item);
            
            item.appendTo(container);
        });
    });

    function enableEditing(button) {
        var container = $('#tiles');
        container.sortable({
          animation: 150, // ms, animation speed moving items when sorting, `0` — without animation
          draggable: ".tile",
          ghostClass: "ghost"
        });

        $('.tile').addClass('editable');
        $('#discard').show();
        button.html('Save');
    }

    function serialiseTiles()
    {
        var tiles = [];
        return tiles;
    }

    function saveTiles(button) {
        var container = $('#tiles');
        var tile_data = $('.tile').map(function() {
            return $(this).data();
        }).get();

        console.log(tile_data);

        $.postJSON(
            'tiles', tile_data
        ).done(function(data, textStatus) {
            $('.tile').removeClass('editable');
            $('#discard').hide();
            container.sortable('destroy');
            button.html('Edit...');
        });
    }

    $("#edit").on("click", function(event) {
        var button = $( event.target );
        if (button.html() == 'Edit...') {
            enableEditing(button);
        } else {
            saveTiles(button);
        }
    });

    $("#discard").on("click", function(event) {
        // FIXME: better way to do this?
        window.location = window.location;
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

});

