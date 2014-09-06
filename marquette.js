/**
 * Copyright 2014 Nicholas Humfrey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

var http = require('http');
var util = require("util");
var express = require("express");
var nopt = require("nopt");
var path = require("path");
var fs = require("fs");
var mqtt = require('mqtt');

// Middleware
var bodyParser = require('body-parser');
var errorhandler = require('errorhandler');
var morgan  = require('morgan');

var app = express();
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({extended: false}));
app.use(morgan());

// development only
if (process.env.NODE_ENV === 'development') {
  app.use(errorhandler());
}


var knownOpts = {
    "settings":[path],
    "v": Boolean,
    "help": Boolean
};
var shortHands = {
    "s":["--settings"],
    "?":["--help"]
};
nopt.invalidHandler = function(k,v,t) {
    console.log(k,v,t);
}

var parsedArgs = nopt(knownOpts,shortHands,process.argv,2)

if (parsedArgs.help) {
    console.log("Marquette");
    console.log("Usage: node marquette.js [-v] [-?] [--settings settings.js]");
    console.log("");
    console.log("Options:");
    console.log("  -s, --settings FILE  use specified settings file");
    console.log("  -v                   enable verbose output");
    console.log("  -?, --help           show usage");
    process.exit();
}

var settingsFile = parsedArgs.settings || "./settings";
try {
    var settings = require(settingsFile);
} catch(err) {
    if (err.code == 'MODULE_NOT_FOUND') {
        console.log("Unable to load settings file "+settingsFile);
    } else {
        console.log(err);
    }
    process.exit();
}

if (parsedArgs.v) {
    settings.verbose = true;
}

settings.uiHost = settings.uiHost||"0.0.0.0";
settings.uiPort = settings.uiPort||1890;
settings.mqttHost = settings.mqttHost||"127.0.0.1";
settings.mqttPort = settings.mqttPort||1883;

process.on('uncaughtException',function(err) {
    util.log('[marquette] Uncaught Exception:');
    util.log(err.stack);
    process.exit(1);
});

process.on('SIGINT', function () {
    process.exit();
});








browsers = [];
topics = {};
tiles = require('./tiles.json');


// Connect to the MQTT sever
client = mqtt.createClient(settings.mqttPort, settings.mqttHost);
client.subscribe('test');

// When a message is received from the MQTT server, pass it on to the browsers
client.on('message', function(topic, payload) {
    console.log("Received MQTT: "+payload);
    topics[topic] = payload;

    browsers.forEach(function(res) {
        console.log("  informing browser: "+res);
        res.write("data: " + JSON.stringify({topic:topic, payload:payload}) + "\n\n");
    });
});


// Send a ping every 20 seconds to the browsers to keep the HTTP connections open
setInterval(function() {
    browsers.forEach(function(res) {
        res.write(": ping\n\n");
    });
}, 20000);


app.get('/tiles', function(req, res) {
  res.send(tiles);
});

// FIXME: this should be a PUT
app.post('/tiles', function(req, res) {
    fs.writeFile('tiles.json', JSON.stringify(req.body, null, 4), function(err) {
        if(err) {
          console.log(err);
          res.status(500).send(err);
        } else {
          res.status(204).end();
        }
    });
});

app.get('/update-stream', function(req, res) {
    req.socket.setNoDelay(true);
    res.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache'
    });
    res.write(":\n");

    browsers.push(res);

    req.on("close", function() {
        index = browsers.indexOf(req);
        browsers.splice(index, 1);
    });
});

app.get('/topics', function(req, res) {
  res.send(topics);
});

app.get('/topics/*', function(req, res) {
    var topic = req.params[0];
    res.type('text/plain');
    if (topic in topics) {
        res.status(200).send(topics[topic]);
    } else {
        res.status(404).send('Topic Not Found');
    }
});

app.post('/topics/*', function(req, res) {
    var topic = req.params[0];
    console.log("Publishing: '"+req.body.payload+"' to '"+topic+"'");
    client.publish(topic, req.body.payload);
    res.status(204).end();
});


// Serve a static file, if it exists
app.use(express.static(path.join(__dirname, 'public')));


var server = http.createServer(app)
server.listen(settings.uiPort, settings.uiHost, function(){
    console.log('Express server listening on port ' + settings.uiPort);
});
