
/**
 * Module dependencies.
 */

var express = require('express');
var http = require('http');
var mqtt = require('mqtt');
var path = require('path');

var app = express();

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.use(express.favicon());
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(app.router);
app.use(express.static(path.join(__dirname, 'public')));

// development only
if ('development' == app.get('env')) {
  app.use(express.errorHandler());
}


browsers = [];

// Connect to the MQTT sever
client = mqtt.createClient(1883, 'test.mosquitto.org');
client.subscribe('test');

// When a message is received from the MQTT server, pass it on to the browsers
client.on('message', function(topic, payload) {
  console.log("Received MQTT: "+payload);

  browsers.forEach(function(res) {
    console.log("  informing browser: "+res);
    res.write("data: " + JSON.stringify({topic:topic, payload:payload}) + "\n\n");
  });
});

// Send a ping every 25 seconds to the browsers to keep the HTTP connections open
setInterval(function() {
  browsers.forEach(function(res) {
    res.write(": ping\n\n");
  });
}, 25000);


app.get('/update-stream', function(req, res) {
  req.socket.setTimeout(Infinity);

  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache'
  });
  res.write("\n");
  browsers.push(res);

  req.on("close", function() {
    index = browsers.indexOf(req);
    browsers.splice(index, 1);
  });

});


app.post('/topics/:topic', function(req, res) {
  console.log("Publishing: "+req.body.payload);
  client.publish(req.params.topic, req.body.payload);
  res.send(204);
});


app.use("/",express.static(__dirname + '/../public'));


http.createServer(app).listen(app.get('port'), function(){
  console.log('Express server listening on port ' + app.get('port'));
});
