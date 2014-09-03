var columns = 4;
var rows = 4;
var min_margin = 6;

$(function(){ //DOM Ready

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
                var button = $('<button>'+tile.name+'</button>').attr('type', 'button');
                button.on("click", function(event) {
                    $.ajax({
                        url: '/topics/'+tile.topic,
                        type: 'post',
                        dataType: 'json',
                        data: {payload: tile.payload}
                    });
                });
                button.appendTo(div);
            } else if (tile.type == 'text') {
                div.html(
                  '<label>'+tile.name+'</label>'+
                  '<div class="text"></div>'
                );
            } else {
                div.html("Unknown tile type: "+tile.type);
            }

            gridster.add_widget(div, size_x=1, size_y=1, tile.col, tile.row);
        });
    });


    $("#edit").on("click", function(event) {
        var target = $( event.target );
        if (target.html() == 'Edit...') {
            $( '.tile' ).addClass('editable');
            gridster.enable();
            target.html('Save');
        } else {
            $( '.tile' ).removeClass('editable');
            gridster.disable();
            target.html('Edit...');
        }
    });


    $( window ).resize(function() {
        var gridster = $('.gridster').data('gridster');
        if (gridster) {
            gridster.destroy();
        }
        init_gridster();
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
          if (obj.topic == tile.data('topic')) {
              var text = tile.find('.text');
              if (text) {
                  text.html(obj.payload);
              }
          }
      });
    };



    function getTimeStamp() {
        var now = new Date();
        return (now.getFullYear() + '-' +
               (now.getMonth() + 1) + '-' +
               (now.getDate()) + " " +
                now.getHours() + ':' +
              ((now.getMinutes() < 10)
                 ? ("0" + now.getMinutes())
                 : (now.getMinutes())) + ':' +
              ((now.getSeconds() < 10)
                 ? ("0" + now.getSeconds())
                 : (now.getSeconds())));
    }

});

