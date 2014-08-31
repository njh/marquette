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

module.exports = {
    // By default, Marquette accepts connections on all IPv4 interfaces.
    // The following property can be used to listen on a specific interface. For
    // example, the following would only allow connections from the local machine.
    //uiHost: "127.0.0.1",

    // the tcp port that the Marquette web server is listening on
    uiPort: 1890,

    // the hostname of the MQTT broker to send and receive messages to
    mqttHost: "localhost",

    // the tcp port that the MQTT broker listening on
    //mqttPort: 1883,

    // Retry time in milliseconds for MQTT connections
    mqttReconnectTime: 15000

    // Enabled verbose logging output
    //verbose: true
}
